// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smminiresampler.hh"
#include "smfft.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

/* this generates noise signals for any sample rate >= 48000 with noise in frequency range [0,24000] */
class NoiseGenerator
{
  vector<float> noise_96000;
public:
  vector<float> gen_noise (int sr);
};

vector<float>
NoiseGenerator::gen_noise (int sr)
{
  assert (sr >= 48000);

  Random random;
  if (!noise_96000.size())
    {
      const size_t FFT_SIZE = 512 * 1024; // should be power of two, greater than 5 seconds at 96000 (480000)

      float *in = FFT::new_array_float (FFT_SIZE);
      float *out = FFT::new_array_float (FFT_SIZE);

      const double MAG = sqrt (0.5);
      for (size_t i = 0; i < FFT_SIZE; i++)
        in[i] = random.random_double_range (-MAG, MAG);

      vector<float> old_in (in, in+FFT_SIZE);

      FFT::fftar_float (FFT_SIZE, in, out, FFT::PLAN_ESTIMATE);

      // band limit noise spectrum, highest permitted frequency: 24000 Hz
      for (size_t i = 0; i < FFT_SIZE / 2; i++)
        {
          if (i >= FFT_SIZE / 4)
            out[i * 2] = out[i * 2 + 1] = 0;
        }

      FFT::fftsr_float (FFT_SIZE, out, in, FFT::PLAN_ESTIMATE);

      // normalization, output
      noise_96000.resize (FFT_SIZE);
      for (size_t i = 0; i < FFT_SIZE; i++)
        noise_96000[i] = in[i] / FFT_SIZE;

      FFT::free_array_float (in);
      FFT::free_array_float (out);
    }

  vector<float> out (5 * sr);
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, 96000, 440, noise_96000.size(), &noise_96000[0], NULL);
  MiniResampler mini_resampler (dhandle, 96000 / double (sr));
  mini_resampler.read (0, out.size(), &out[0]);
  return out;
}

void
avg_spectrum (const char *label, vector<float>& signal, int sr)
{
  const size_t block_size = 2048;
  size_t offset = 0;

  vector<float> avg (block_size / 2 + 1);
  vector<float> window (block_size);

  for (guint i = 0; i < window.size(); i++)
    window[i] = bse_window_cos (2.0 * i / block_size - 1.0);

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
    printf ("%s %g %g\n", label, i * double (sr) / block_size, avg[i]);
}


size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

void
encode (vector<float>& audio_in, int sr)
{
  EncoderParams enc_params;
  enc_params.mix_freq = sr;
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

void
dump_noise_envelope()
{
  WavSet wav_set;
  Bse::Error error = wav_set.load ("testnoisesr.tmp.smset");
  assert (error == 0);

  assert (wav_set.waves.size() == 1);
  Audio *audio = wav_set.waves[0].audio;

  vector<float> noise_env (32);
  for (size_t pos = 0; pos < audio->contents.size(); pos++)
    {
      AudioBlock *block = &audio->contents[pos];
      assert (noise_env.size() == block->noise.size());
      for (size_t i = 0; i < block->noise.size(); i++)
        noise_env[i] += block->noise_f (i);
    }
  for (size_t i = 0; i < noise_env.size(); i++)
    printf ("noise-envelope %zd %.17g\n", i, noise_env[i] / audio->contents.size());
}

int
main (int argc, char **argv)
{
  Random random;

  sm_init (&argc, &argv);

  int sr = (argc == 2) ? atoi (argv[1]) : 48000;
  printf ("# sr = %d\n", sr);

  NoiseGenerator noise_generator;
  vector<float> noise;
  vector<float> noise_48000;
  if (sr == 48000)
    {
      noise.resize (sr * 5);
      for (size_t i = 0; i < noise.size(); i++)
        noise[i] = random.random_double_range (-0.5, 0.5);
      noise_48000 = noise;
    }
  else
    {
      noise       = noise_generator.gen_noise (sr);
      noise_48000 = noise_generator.gen_noise (48000);
    }

  encode (noise, sr);
  dump_noise_envelope();

  printf ("noise-in-energy %.17g\n", energy (noise_48000));

  vector<float> audio_out (noise_48000.size());
  for (int i = 48000; i < 197000; i += 12000)
    {
      decode (audio_out, i);
      printf ("noise-out-energy %d %.17g\n", i, energy (audio_out));
    }
}
