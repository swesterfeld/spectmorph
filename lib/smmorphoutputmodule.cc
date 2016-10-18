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
MorphOutputModule::set_config (MorphOperator *op)
{
  MorphOutput *out_op = dynamic_cast <MorphOutput *> (op);
  g_return_if_fail (out_op != NULL);

  clear_dependencies();
  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      MorphOperatorModule *mod = NULL;
      EffectDecoder *dec = NULL;

      MorphOperator *op = out_op->channel_op (ch);
      if (op)
        mod = morph_plan_voice->module (op);

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

      // update dec sines & noise
      if (dec)
        {
          dec->enable_sines (out_op->sines());
          dec->enable_noise (out_op->noise());
          dec->enable_chorus (out_op->chorus());
        }

      out_ops[ch] = mod;
      out_decoders[ch] = dec;

      add_dependency (mod);
    }
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
recursive_update_value (MorphOperatorModule *module, double time_ms)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_update_value (deps[i], time_ms);

  if (!module->update_value_tag())
    {
      module->update_value (time_ms);
      module->update_value_tag()++;
    }
}

static void
recursive_reset_value (MorphOperatorModule *module)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_reset_value (deps[i]);

  if (!module->update_value_tag())
    {
      module->reset_value();
      module->update_value_tag()++;
    }
}

void
MorphOutputModule::process (size_t n_samples, float **values, size_t n_ports)
{
  g_return_if_fail (n_ports <= out_decoders.size());

  for (size_t port = 0; port < n_ports; port++)
    {
      if (values[port])
        {
          if (out_decoders[port])
            {
              out_decoders[port]->process (n_samples, 0, 0, values[port]);
            }
          else
            {
              zero_float_block (n_samples, values[port]);
            }
        }
    }
  recursive_reset_tag (this);
  recursive_update_value (this, n_samples / morph_plan_voice->mix_freq() * 1000);
}

void
MorphOutputModule::retrigger (int channel, float freq, int midi_velocity)
{
  for (size_t port = 0; port < CHANNEL_OP_COUNT; port++)
    {
      if (out_decoders[port])
        {
          out_decoders[port]->retrigger (channel, freq, midi_velocity, morph_plan_voice->mix_freq());
        }
    }
  recursive_reset_tag (this);
  recursive_reset_value (this);
}
