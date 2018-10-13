// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourcemodule.hh"
#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include "../instedit/sminstrument.hh"
#include "../instedit/smwavsetbuilder.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSourceModule");

static float
freq_to_note (float freq)
{
  return 69 + 12 * log (freq / 440) / log (2);
}

void
InstrumentSource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  std::lock_guard<std::mutex> lg (mutex);

  Audio  *best_audio = nullptr;
  WavSet *wav_set = &this->wav_set;
  float   best_diff  = 1e10;

  if (wav_set)
    {
      float note = freq_to_note (freq);
      for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel &&
                       wi->velocity_range_min <= midi_velocity &&
                       wi->velocity_range_max >= midi_velocity)
            {
              float audio_note = freq_to_note (audio->fundamental_freq);
              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }
  active_audio = best_audio;  // FIXME: this pointer could become dangling if the underlying wav_set changes
}

Audio*
InstrumentSource::audio()
{
  return active_audio;
}

AudioBlock *
InstrumentSource::audio_block (size_t index)
{
  if (active_audio && index < active_audio->contents.size())
    return &active_audio->contents[index];
  else
    return nullptr;
}


MorphWavSourceModule::MorphWavSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
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
MorphWavSourceModule::set_config (MorphOperator *op)
{
  MorphWavSource *source = dynamic_cast<MorphWavSource *> (op);

  printf ("MorphWavSourceModule::set_config: using instrument=%s\n", source->instrument().c_str());
  Instrument inst;
  inst.load (source->instrument());
  printf ("%zd\n", inst.size());
  WavSetBuilder builder (&inst);
  builder.run();

  std::lock_guard<std::mutex> lg (my_source.mutex);
  builder.get_result (my_source.wav_set);
  printf ("wsize %zd\n", my_source.wav_set.waves.size());
}

#include "../instedit/smwavsetbuilder.cc"
