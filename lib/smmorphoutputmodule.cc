// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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

  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      EffectDecoder *dec = NULL;

      MorphOperatorModule *mod = morph_plan_voice->module (cfg->channel_ops[ch]);

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
              dec = new EffectDecoder (this, mod->source());
            }
        }

      if (dec)
        dec->set_config (cfg, morph_plan_voice->mix_freq());

      out_ops[ch] = mod;
      out_decoders[ch] = dec;
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

float
MorphOutputModule::filter_cutoff_mod() const
{
  return apply_modulation (cfg->filter_cutoff_mod);
}

float
MorphOutputModule::filter_resonance_mod() const
{
  return apply_modulation (cfg->filter_resonance_mod);
}

float
MorphOutputModule::filter_mix_mod() const
{
  return apply_modulation (cfg->filter_mix_mod);
}

void
MorphOutputModule::process (const TimeInfo& time_info, size_t n_samples, float **values, size_t n_ports, const float *freq_in)
{
  g_return_if_fail (n_ports <= out_decoders.size());

  const bool have_cycle = morph_plan_voice->morph_plan_synth()->have_cycle();

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
  const bool have_cycle = morph_plan_voice->morph_plan_synth()->have_cycle();
  if (have_cycle)
    return;

  for (size_t port = 0; port < CHANNEL_OP_COUNT; port++)
    {
      if (out_decoders[port])
        {
          out_decoders[port]->retrigger (channel, freq, midi_velocity, morph_plan_voice->mix_freq());
        }
    }
  morph_plan_voice->reset_value (time_info);
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
