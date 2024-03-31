// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphlinearmodule.hh"
#include "smmorphlinear.hh"
#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"
#include "smleakdebugger.hh"
#include "smlivedecoder.hh"
#include "smmorphutils.hh"
#include "smutils.hh"
#include "smrtmemory.hh"
#include <glib.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::min;
using std::max;
using std::sort;

static LeakDebugger leak_debugger ("SpectMorph::MorphLinearModule");

#define DEBUG (0)

MorphLinearModule::MorphLinearModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;

  audio.fundamental_freq     = 440;
  audio.mix_freq             = 48000;
  audio.frame_size_ms        = 1;
  audio.frame_step_ms        = 1;
  audio.zeropad              = 4;
  audio.loop_type            = Audio::LOOP_NONE;

  leak_debugger.add (this);
}

MorphLinearModule::~MorphLinearModule()
{
  leak_debugger.del (this);
}

void
MorphLinearModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphLinear::Config *> (op_cfg);
  g_return_if_fail (cfg != NULL);

  left_mod = morph_plan_voice->module (cfg->left_op);
  right_mod = morph_plan_voice->module (cfg->right_op);

  have_left_source = cfg->left_wav_set != nullptr;
  if (have_left_source)
    left_source.set_wav_set (cfg->left_wav_set);

  have_right_source = cfg->right_wav_set != nullptr;
  if (have_right_source)
    right_source.set_wav_set (cfg->right_wav_set);
}

void
MorphLinearModule::MySource::retrigger (int channel, float freq, int midi_velocity)
{
  if (module->left_mod && module->left_mod->source())
    {
      module->left_mod->source()->retrigger (channel, freq, midi_velocity);
    }

  if (module->right_mod && module->right_mod->source())
    {
      module->right_mod->source()->retrigger (channel, freq, midi_velocity);
    }

  if (module->have_left_source)
    {
      module->left_source.retrigger (channel, freq, midi_velocity);
    }

  if (module->have_right_source)
    {
      module->right_source.retrigger (channel, freq, midi_velocity);
    }
}

void
MorphLinearModule::MySource::set_portamento_freq (float freq)
{
  if (module->left_mod && module->left_mod->source())
    module->left_mod->source()->set_portamento_freq (freq);

  if (module->right_mod && module->right_mod->source())
    module->right_mod->source()->set_portamento_freq (freq);

  if (module->have_left_source)
    module->left_source.set_portamento_freq (freq);

  if (module->have_right_source)
    module->right_source.set_portamento_freq (freq);
}

Audio*
MorphLinearModule::MySource::audio()
{
  return &module->audio;
}

bool
MorphLinearModule::MySource::rt_audio_block (size_t index, RTAudioBlock& out_audio_block)
{
  bool have_left = false, have_right = false;

  const double morphing = module->apply_modulation (module->cfg->morphing_mod);
  const double time_ms = index; // 1ms frame step
  const auto   morph_mode = module->cfg->db_linear ? MorphUtils::MorphMode::DB_LINEAR : MorphUtils::MorphMode::LINEAR;

  RTAudioBlock left_block (module->rt_memory_area()), right_block (module->rt_memory_area());

  if (module->left_mod && module->left_mod->source())
    {
      have_left = MorphUtils::get_normalized_block (module->left_mod->source(), time_ms, left_block);
    }

  if (module->right_mod && module->right_mod->source())
    {
      have_right = MorphUtils::get_normalized_block (module->right_mod->source(), time_ms, right_block);
    }

  if (module->have_left_source)
    {
      have_left = MorphUtils::get_normalized_block (&module->left_source, time_ms, left_block);
    }

  if (module->have_right_source)
    {
      have_right = MorphUtils::get_normalized_block (&module->right_source, time_ms, right_block);
    }

  return MorphUtils::morph (out_audio_block, have_left, left_block, have_right, right_block, morphing, morph_mode);

}

LiveDecoderSource *
MorphLinearModule::source()
{
  return &my_source;
}
