// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphwavsourcemodule.hh"
#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include "sminstrument.hh"
#include "smwavsetbuilder.hh"
#include "smmorphplanvoice.hh"
#include <glib.h>
#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSourceModule");

void
MorphWavSourceModule::InstrumentSource::retrigger (int channel, float freq, int midi_velocity)
{
  Audio  *best_audio = nullptr;
  float   best_diff  = 1e10;

  WavSet *wav_set = project->get_wav_set (object_id);
  if (wav_set)
    {
      float note = sm_freq_to_note (freq);
      for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel &&
                       wi->velocity_range_min <= midi_velocity &&
                       wi->velocity_range_max >= midi_velocity)
            {
              float audio_note = sm_freq_to_note (audio->fundamental_freq);
              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }
  active_audio = best_audio;
}

Audio*
MorphWavSourceModule::InstrumentSource::audio()
{
  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;

  return active_audio;
}

bool
MorphWavSourceModule::InstrumentSource::rt_audio_block (size_t index, RTAudioBlock& out_block)
{
  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;

  if (active_audio && module->cfg->play_mode == MorphWavSource::PLAY_MODE_CUSTOM_POSITION)
    {
      const double position = module->apply_modulation (module->cfg->position_mod) * 0.01;

      int start, end;
      if (active_audio->loop_type == Audio::LOOP_NONE)
        {
          // play everything
          start = 0;
          end = active_audio->contents.size() - 1;
        }
      else
        {
          // play loop
          start = active_audio->loop_start;
          end = active_audio->loop_end;
        }
      index = sm_bound (start, sm_round_positive ((1 - position) * start + position * end), end);
    }
  if (active_audio && index < active_audio->contents.size())
    {
      out_block.assign (active_audio->contents[index]);
      return true;
    }
  else
    {
      return false;
    }
}

void
MorphWavSourceModule::InstrumentSource::update_project_and_object_id (Project *new_project, int new_object_id)
{
  project = new_project;
  object_id = new_object_id;

  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;
}

MorphWavSourceModule::MorphWavSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;

  leak_debugger.add (this);
}

MorphWavSourceModule::~MorphWavSourceModule()
{
  leak_debugger.del (this);
}

LiveDecoderSource *
MorphWavSourceModule::source()
{
  return &my_source;
}

void
MorphWavSourceModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphWavSource::Config *> (op_cfg);

  my_source.update_project_and_object_id (cfg->project, cfg->object_id);
}
