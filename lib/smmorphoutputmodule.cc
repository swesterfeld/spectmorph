// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputmodule.hh"
#include "smmorphoutput.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include <glib.h>
#include <assert.h>

#define CHANNEL_OP_COUNT 4

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphOutputModule");

MorphOutputModule::MorphOutputModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  out_ops.resize (CHANNEL_OP_COUNT);
  out_decoders.resize (CHANNEL_OP_COUNT);

  leak_debugger.add (this);
}

MorphOutputModule::~MorphOutputModule()
{
  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      if (out_decoders[ch])
        {
          delete out_decoders[ch];
          out_decoders[ch] = NULL;
        }
    }
  leak_debugger.del (this);
}

void
MorphOutputModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphOutput::Config *> (op_cfg);
  g_return_if_fail (cfg != NULL);

  clear_dependencies();
  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      EffectDecoder *dec = NULL;

      MorphOperatorModule *mod = morph_plan_voice->module (cfg->channel_ops[ch].ptr_id());

      if (mod == out_ops[ch]) // same source
        {
          dec = out_decoders[ch];
          // keep decoder as it is
        }
      else
        {
          if (out_decoders[ch])
            delete out_decoders[ch];
          if (mod)
            {
              dec = new EffectDecoder (mod->source());
            }
        }

      if (dec)
        dec->set_config (cfg, morph_plan_voice->mix_freq());

      out_ops[ch] = mod;
      out_decoders[ch] = dec;

      add_dependency (mod);
    }
}

bool
MorphOutputModule::portamento() const
{
  return cfg->portamento;
}

float
MorphOutputModule::portamento_glide() const
{
  return cfg->portamento_glide;
}

float
MorphOutputModule::velocity_sensitivity() const
{
  return cfg->velocity_sensitivity;
}

static void
recursive_reset_tag (MorphOperatorModule *module)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_reset_tag (deps[i]);

  module->update_value_tag() = 0;
}

static void
recursive_reset_value (MorphOperatorModule *module, const TimeInfo& time_info)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_reset_value (deps[i], time_info);

  if (!module->update_value_tag())
    {
      module->reset_value (time_info);
      module->update_value_tag()++;
    }
}

static bool
recursive_cycle_check (MorphOperatorModule *module, int depth = 0)
{
  /* check if processing would fail due to cycles
   *
   * this check should avoid crashes in this situation, although no audio will be produced
   */
  if (depth > 500)
    return true;

  for (auto mod : module->dependencies())
    if (recursive_cycle_check (mod, depth + 1))
      return true;

  return false;
}

void
MorphOutputModule::process (const TimeInfo& time_info, size_t n_samples, float **values, size_t n_ports, const float *freq_in)
{
  g_return_if_fail (n_ports <= out_decoders.size());

  const bool have_cycle = recursive_cycle_check (this);
  block_time = time_info;

  for (size_t port = 0; port < n_ports; port++)
    {
      if (values[port])
        {
          if (out_decoders[port] && !have_cycle)
            {
              out_decoders[port]->process (n_samples, freq_in, values[port]);
            }
          else
            {
              zero_float_block (n_samples, values[port]);
            }
        }
    }
}

TimeInfo
MorphOutputModule::compute_time_info() const
{
  /* this is not really correct, but as long as we only have one decoder, it should work */
  for (auto dec : out_decoders)
    {
      TimeInfo time_info = block_time;

      time_info.time_ms += dec->time_offset_ms();
      /* we don't even try to correct time_info.ppq_pos here, because doing so might
       * introduce backwards-jumps triggered by tempo changes; the resolution of
       * ppq_pos (once per block) should be sufficient in practice
       */
      return time_info;
    }
  return block_time;
}

void
MorphOutputModule::retrigger (const TimeInfo& time_info, int channel, float freq, int midi_velocity)
{
  if (recursive_cycle_check (this))
    return;

  for (size_t port = 0; port < CHANNEL_OP_COUNT; port++)
    {
      if (out_decoders[port])
        {
          out_decoders[port]->retrigger (channel, freq, midi_velocity, morph_plan_voice->mix_freq());
        }
    }
  recursive_reset_tag (this);
  recursive_reset_value (this, time_info);
}

void
MorphOutputModule::release()
{
  for (auto dec : out_decoders)
    {
      if (dec)
        dec->release();
    }
}

bool
MorphOutputModule::done()
{
  // done means: the signal will be only zeros from here
  bool done = true;

  for (auto dec : out_decoders)
    {
      // we're done if all decoders are done
      if (dec)
        done = done && dec->done();
    }
  return done;
}
