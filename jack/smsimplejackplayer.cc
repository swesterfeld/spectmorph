// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smsimplejackplayer.hh"
#include "smlivedecodersource.hh"
#include <unistd.h>

using namespace SpectMorph;

using std::vector;
using std::max;

namespace {

class Source : public LiveDecoderSource
{
  Audio *my_audio;
public:
  Source (Audio *my_audio);

  void retrigger (int, float, int);
  Audio* audio();
  bool rt_audio_block (size_t index, RTAudioBlock& out_block) override;
};

Source::Source (Audio *audio) :
  my_audio (audio)
{
}

void
Source::retrigger (int, float, int)
{
}

Audio *
Source::audio()
{
  return my_audio;
}

bool
Source::rt_audio_block (size_t index, RTAudioBlock& out_block)
{
  if (my_audio && index < my_audio->contents.size())
    {
      out_block.assign (my_audio->contents[index]);
      return true;
    }
  else
    {
      return false;
    }
}

}

static int
jack_process (jack_nframes_t nframes, void *arg)
{
  SimpleJackPlayer *instance = reinterpret_cast<SimpleJackPlayer *> (arg);
  return instance->process (nframes);
}

SimpleJackPlayer::SimpleJackPlayer (const std::string& client_name) :
  decoder (NULL),
  decoder_audio (NULL),
  decoder_source (NULL),
  decoder_volume (0), // must be initialized later on
  decoder_fade_out (false)
{
  jack_client = jack_client_open (client_name.c_str(), JackNullOption, NULL);

  jack_set_process_callback (jack_client, jack_process, this);

  audio_out_port = jack_port_register (jack_client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  if (jack_activate (jack_client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
  jack_mix_freq = jack_get_sample_rate (jack_client);
}

int
SimpleJackPlayer::process (jack_nframes_t nframes)
{
  std::lock_guard<std::mutex> lock (decoder_mutex);

  float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (audio_out_port, nframes);
  if (decoder)
    {
      decoder->process (*decoder_rt_memory_area, nframes, nullptr, audio_out);
      for (size_t i = 0; i < nframes; i++)
        audio_out[i] *= decoder_volume;
    }
  else
    {
      for (size_t i = 0; i < nframes; i++)
        {
          audio_out[i] = 0;
        }
    }
  // fade out needs to be done in any case, as the other thread will wait for completion
  if (decoder_fade_out)
    {
      const double fade_out_samples = 0.03 * jack_mix_freq; // 30 ms fadeout time
      const double delta = 1 / fade_out_samples;

      for (size_t i = 0; i < nframes; i++)
        {
          decoder_fade_out_level = max (decoder_fade_out_level - delta, 0.0);
          audio_out[i] *= decoder_fade_out_level;
        }
    }
  return 0;
}

void
SimpleJackPlayer::play (Audio *audio, bool use_samples)
{
  RTMemoryArea      *new_decoder_rt_memory_area = nullptr;
  LiveDecoder       *new_decoder        = NULL;
  Audio             *new_decoder_audio  = NULL;
  LiveDecoderSource *new_decoder_source = NULL;

  // fade out old decoder to avoid clicks
  fade_out_blocking();

  if (audio)
    {
      // create a deep copy, so that JACK thread can access data in JACK thread
      new_decoder_audio = audio->clone();

      new_decoder_source = new Source (new_decoder_audio);
      new_decoder = new LiveDecoder (new_decoder_source, jack_mix_freq);
      new_decoder_rt_memory_area = new RTMemoryArea();

      new_decoder->enable_original_samples (use_samples);
      new_decoder->retrigger (/* channel */ 0, audio->fundamental_freq, 127);

      // touch decoder in non-RT-thread to precompute tables & co
      vector<float> samples (10000);
      new_decoder->process (*new_decoder_rt_memory_area, samples.size(), nullptr, &samples[0]);

      // finally setup decoder for JACK thread
      new_decoder->retrigger (/* channel */ 0, audio->fundamental_freq, 127);
    }
  update_decoder (new_decoder_rt_memory_area, new_decoder, new_decoder_audio, new_decoder_source);
}

void
SimpleJackPlayer::stop()
{
  play (NULL, true);
}

void
SimpleJackPlayer::update_decoder (RTMemoryArea *new_decoder_rt_memory_area, LiveDecoder *new_decoder, Audio *new_decoder_audio, LiveDecoderSource *new_decoder_source)
{
  RTMemoryArea      *old_decoder_rt_memory_area;
  LiveDecoder       *old_decoder;
  Audio             *old_decoder_audio;
  LiveDecoderSource *old_decoder_source;

  /* setup new player objects for JACK thread */
  decoder_mutex.lock();

  old_decoder_rt_memory_area = decoder_rt_memory_area;
  old_decoder = decoder;
  old_decoder_source = decoder_source;
  old_decoder_audio = decoder_audio;

  decoder_rt_memory_area = new_decoder_rt_memory_area;
  decoder = new_decoder;
  decoder_source = new_decoder_source;
  decoder_audio = new_decoder_audio;
  decoder_fade_out = false;

  decoder_mutex.unlock();

  /* delete old (no longer needed) player objects */
  if (old_decoder_rt_memory_area)
    delete old_decoder_rt_memory_area;
  if (old_decoder)
    delete old_decoder;
  if (old_decoder_audio)
    delete old_decoder_audio;
  if (old_decoder_source)
    delete old_decoder_source;
}

void
SimpleJackPlayer::set_volume (double new_volume)
{
  std::lock_guard<std::mutex> lock (decoder_mutex);
  decoder_volume = new_volume;
}

SimpleJackPlayer::~SimpleJackPlayer()
{
  jack_client_close (jack_client);

  // delete old decoder objects (if any)
  update_decoder (nullptr, nullptr, nullptr, nullptr);
}

void
SimpleJackPlayer::fade_out_blocking()
{
  // start fadeout
  decoder_mutex.lock();
  if (!decoder_fade_out)
    {
      decoder_fade_out_level = 1;
      decoder_fade_out = true;
    }
  decoder_mutex.unlock();

  // wait for fade out level to reach zero
  bool done = false;
  int  wait_ms = 0;
  while (!done)
    {
      usleep (10 * 1000);
      wait_ms += 10;

      if (wait_ms > 500)
        {
          fprintf (stderr, "SimpleJackPlayer::fade_out_blocking(): timeout waiting for jack thread\n");
          return;
        }

      decoder_mutex.lock();
      done = (decoder_fade_out_level == 0);
      decoder_mutex.unlock();
    }
}

double
SimpleJackPlayer::mix_freq() const
{
  return jack_mix_freq;
}
