// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smminiresampler.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

void
encode (vector<float>& audio_in)
{
  EncoderParams enc_params;
  enc_params.mix_freq = 48000;
  enc_params.zeropad = 4;
  enc_params.fundamental_freq = 440;
  enc_params.frame_size_ms = 40;
  enc_params.frame_step_ms = enc_params.frame_size_ms / 4.0;
  enc_params.frame_size = make_odd (enc_params.mix_freq * 0.001 * enc_params.frame_size_ms);
  enc_params.frame_step = enc_params.mix_freq * 0.001 * enc_params.frame_step_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < enc_params.frame_size)
    block_size *= 2;
  enc_params.block_size = block_size;
  vector<float> window (block_size);

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < enc_params.frame_size)
        window[i] = bse_window_cos (2.0 * i / enc_params.frame_size - 1.0);
      else
        window[i] = 0;
    }

  Encoder encoder (enc_params);

  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, enc_params.mix_freq, 440, audio_in.size(), &audio_in[0], NULL);
  Bse::Error error = gsl_data_handle_open (dhandle);
  assert (error == 0);

  const char *sm_file = "testnoisesr.tmp.sm";
  encoder.encode (dhandle, 0, window, 1, /*attack*/ false, /*sines*/ false, /*lpc*/ false);
  encoder.save (sm_file);

  WavSet wav_set;

  WavSetWave new_wave;
  new_wave.midi_note = 60; // doesn't matter
  new_wave.channel = 0;
  new_wave.path = sm_file;
  wav_set.waves.push_back (new_wave);

  wav_set.save ("testnoisesr.tmp.smset", true);
}

void
decode (vector<float>& audio_out, int sr)
{
  WavSet wav_set;
  Bse::Error error = wav_set.load ("testnoisesr.tmp.smset");
  assert (error == 0);

  LiveDecoder decoder (&wav_set);

  vector<float> audio_high (sr * 5);

  float freq = 440;
  decoder.retrigger (0, freq, 127, sr);
  decoder.process (audio_high.size(), 0, 0, &audio_high[0]);

  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, sr, 440, audio_high.size(), &audio_high[0], NULL);
  MiniResampler mini_resampler (dhandle, double (sr) / 48000.);
  mini_resampler.read (0, audio_out.size(), &audio_out[0]);
}

double
energy (vector<float>& audio)
{
  double e = 0;
  for (size_t i = 0; i < audio.size(); i++)
    e += audio[i] * audio[i];

  return e;
}

int
main (int argc, char **argv)
{
  Random random;

  sm_init (&argc, &argv);

  vector<float> noise (48000 * 5);
  vector<float> audio_out (noise.size());

  for (size_t i = 0; i < noise.size(); i++)
    noise[i] = random.random_double_range (-0.5, 0.5);

  encode (noise);

  printf ("noise-in-energy %.17g\n", energy (noise));

  for (int i = 48000; i < 197000; i += 12000)
    {
      decode (audio_out, i);
      printf ("noise-out-energy %d %.17g\n", i, energy (audio_out));
    }
}
