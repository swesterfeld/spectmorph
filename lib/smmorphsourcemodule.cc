// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphsourcemodule.hh"
#include "smmorphsource.hh"
#include "smmorphplan.hh"
#include "smwavsetrepo.hh"
#include "smleakdebugger.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphSourceModule");

SimpleWavSetSource::SimpleWavSetSource() :
  wav_set (NULL),
  active_audio (NULL)
{
}

void
SimpleWavSetSource::set_wav_set (const string& path)
{
  WavSet *new_wav_set = WavSetRepo::the()->get (path);
  if (new_wav_set != wav_set)
    {
      wav_set = new_wav_set;
      active_audio = NULL;
    }
}

void
SimpleWavSetSource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  Audio *best_audio = NULL;
  float  best_diff  = 1e10;

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
SimpleWavSetSource::audio()
{
  return active_audio;
}

AudioBlock *
SimpleWavSetSource::audio_block (size_t index)
{
  if (active_audio && index < active_audio->contents.size())
    return &active_audio->contents[index];
  else
    return NULL;
}

MorphSourceModule::MorphSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);
}

MorphSourceModule::~MorphSourceModule()
{
  leak_debugger.del (this);
}

LiveDecoderSource *
MorphSourceModule::source()
{
  return &my_source;
}

void
MorphSourceModule::set_config (const MorphOperatorConfig *op_cfg)
{
  auto cfg = dynamic_cast<const MorphSource::Config *> (op_cfg);

  my_source.set_wav_set (cfg->path);
}
