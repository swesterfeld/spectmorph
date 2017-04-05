// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"

#if SPECTMORPH_HAVE_BSE
#include <bse/bsemathsignal.hh>
#include <bse/gsldatautils.hh>
#endif

#include <vector>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace SpectMorph;

using std::vector;

size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

void
encode_decode (vector<float>& audio_in, vector<float>& audio_out)
{
  EncoderParams enc_params;
  enc_params.mix_freq = 44100;
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
        window[i] = window_cos (2.0 * i / enc_params.frame_size - 1.0);
      else
        window[i] = 0;
    }

  Encoder encoder (enc_params);

  WavData wav_data (audio_in, 1, enc_params.mix_freq);

  const char *sm_file = "testnoise.tmp.sm";
  encoder.encode (wav_data, 0, window, 1, /*attack*/ false, /*sines*/ false);
  encoder.save (sm_file);

  WavSet wav_set;
  LiveDecoder decoder (&wav_set);

  WavSetWave new_wave;
  new_wave.midi_note = 60; // doesn't matter
  new_wave.channel = 0;
  new_wave.path = sm_file;
  wav_set.waves.push_back (new_wave);

  wav_set.save ("testnoise.tmp.smset", true);

  wav_set = WavSet();
  SpectMorph::Error error = wav_set.load ("testnoise.tmp.smset");
  assert (error == 0);

  float freq = 440;
  decoder.retrigger (0, freq, 127, enc_params.mix_freq);
  decoder.process (audio_out.size(), 0, 0, &audio_out[0]);
}

void
avg_spectrum (const char *label, vector<float>& signal)
{
  const size_t block_size = 2048;
  size_t offset = 0;

  vector<float> avg (block_size / 2 + 1);
  vector<float> window (block_size);

  for (guint i = 0; i < window.size(); i++)
    window[i] = window_cos (2.0 * i / block_size - 1.0);

  float *in = FFT::new_array_float (block_size);
  float *out = FFT::new_array_float (block_size + 2);

  while (offset + block_size < signal.size())
    {
      std::copy (signal.begin() + offset, signal.begin() + offset + block_size, in);
      for (size_t i = 0; i < block_size; i++)
        in[i] *= window[i];

      FFT::fftar_float (block_size, in, out);

      out[block_size] = out[1];
      out[block_size+1] = 0;
      out[1] = 0;

      for (size_t d = 0; d < block_size + 2; d += 2)
        avg[d/2] += out[d] * out[d] + out[d+1] * out[d+1];
      offset += block_size / 4;
    }

  for (size_t i = 0; i < avg.size(); i++)
    printf ("%s %g %g\n", label, i * 44100.0 / block_size, avg[i]);
}

void
highpass (vector<float>& audio_in, vector<float>& audio_out, double cutoff_freq)
{
#if !SPECTMORPH_HAVE_BSE
  g_printerr ("testnoise: highpass: not supported, need libbse to do this\n");
  g_assert_not_reached();
#else
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, 44100, 440, audio_in.size(), &audio_in[0], NULL);
  Bse::Error error = gsl_data_handle_open (dhandle);
  assert (error == 0);

  GslDataHandle *highpass_dhandle = bse_data_handle_new_fir_highpass (dhandle, cutoff_freq, 64);
  error = gsl_data_handle_open (highpass_dhandle);
  assert (error == 0);

  GslDataPeekBuffer peek_buffer = { 0, };
  audio_out.resize (audio_in.size());
  for (size_t i = 0; i < audio_out.size(); i++)
    audio_out[i] = gsl_data_handle_peek_value (highpass_dhandle, i, &peek_buffer);
#endif
}

void
energy (vector<float>& audio, const char *label)
{
  double e = 0;
  for (size_t i = 0; i < audio.size(); i++)
    e += audio[i] * audio[i];

  printf ("%s %g\n", label, e);
}

int
main (int argc, char **argv)
{
  Random random;

  sm_init (&argc, &argv);

  vector<float> noise (44100 * 5);
  vector<float> audio_out (noise.size());

  for (size_t i = 0; i < noise.size(); i++)
    noise[i] = random.random_double_range (-0.5, 0.5);

  encode_decode (noise, audio_out);

  energy (noise, "white-noise-in-energy");
  energy (audio_out, "white-noise-out-energy");

  avg_spectrum ("white-noise-in", noise);
  avg_spectrum ("white-noise-out", audio_out);

  vector<float> hp_noise;
  highpass (noise, hp_noise, 3000);
  encode_decode (hp_noise, audio_out);

  energy (hp_noise, "hp-noise-in-energy");
  energy (audio_out, "hp-noise-out-energy");

  avg_spectrum ("hp-noise-in", hp_noise);
  avg_spectrum ("hp-noise-out", audio_out);

  unlink ("testnoise.tmp.sm");
  unlink ("testnoise.tmp.smset");
}
