// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourcemodule.hh"
#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include "sminstrument.hh"
#include "smwavsetbuilder.hh"
#include "smcache.hh"
#include <glib.h>
#include <thread>

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
  Audio  *best_audio = nullptr;
  float   best_diff  = 1e10;

  // we can not delete the old wav_set between retrigger() invocations
  //  - LiveDecoder may keep a pointer to contained Audio* entries (which die if the WavSet is freed)

  CacheEntry *cache_entry = Cache::the()->lookup (instrument);

  if (cache_entry)
    wav_set = cache_entry->wav_set();
  else
    wav_set = nullptr;

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
  active_audio = best_audio;
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

void
InstrumentSource::update_instrument (const string& instrument)
{
  this->instrument = instrument;
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
  Cache& cache = *Cache::the();

  CacheEntry *cache_entry = cache.lookup (source->instrument());
  if (!cache_entry)
    {
      cache.store (source->instrument(), new CacheEntry()); // store empty cache entry

      printf ("MorphWavSourceModule::set_config: using instrument=%s source=%ld\n", source->instrument().c_str(), source->instrument_id());

      new std::thread ([filename = source->instrument()]() {
        Instrument inst;
        inst.load (filename);

        /* this should not be necessary if builder was thread safe */
        static std::mutex bmutex;
        std::lock_guard<std::mutex> lg (bmutex);

        WavSetBuilder builder (&inst);
        builder.run();
        CacheEntry *cache_entry = Cache::the()->lookup (filename);

        auto wav_set = std::make_shared<WavSet>();
        builder.get_result (*wav_set);

        cache_entry->set_wav_set (wav_set);
      });

    }
  my_source.update_instrument (source->instrument());
}
