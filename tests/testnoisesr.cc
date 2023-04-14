// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smminiresampler.hh"
#include "smfft.hh"
#include "smdebug.hh"

#include <assert.h>
#include <unistd.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;

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
  WavData wav_data (noise_96000, 1, 96000, 32);
  MiniResampler mini_resampler (wav_data, 96000 / double (sr));
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
encode (vector<float>& audio_in, int sr, const string& win, float fundamental_freq)
{
  EncoderParams enc_params;
  enc_params.mix_freq = sr;
  enc_params.zeropad = 4;
  enc_params.fundamental_freq = fundamental_freq;
  enc_params.frame_size_ms = 40;
  enc_params.frame_size_ms = max<double> (enc_params.frame_size_ms, 1000 / enc_params.fundamental_freq * 4);
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
        {
          if (win == "rect")
            {
              window[i] = 1;
            }
          else if (win == "cos")
            {
              window[i] = window_cos (2.0 * i / enc_params.frame_size - 1.0);
            }
          else
            {
              assert (false); // not reached
            }
        }
      else
        window[i] = 0;
    }
  enc_params.window = window;

  Encoder encoder (enc_params);

  WavData wav_data (audio_in, 1, enc_params.mix_freq, 32);

  const char *sm_file = "testnoisesr.tmp.sm";
  encoder.encode (wav_data, 0, 1, /*attack*/ false, /*sines*/ false);
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
  Error error = wav_set.load ("testnoisesr.tmp.smset");
  assert (!error);

  LiveDecoder decoder (&wav_set, sr);

  float freq = 440;
  decoder.retrigger (0, freq, 127);
  decoder.process (audio_out.size(), nullptr, &audio_out[0]);
}

void
decode_resample (vector<float>& audio_out, int sr)
{
  vector<float> audio_high (sr * 5);

  decode (audio_high, sr);

  WavData wav_data (audio_high, 1, sr, 32);
  MiniResampler mini_resampler (wav_data, double (sr) / 48000.);
  mini_resampler.read (0, audio_out.size(), &audio_out[0]);
}

void
delete_tmp_files()
{
  unlink ("testnoisesr.tmp.smset");
  unlink ("testnoisesr.tmp.sm");
}

double
energy (vector<float>& audio)
{
  double e = 0;
  for (size_t i = 0; i < audio.size(); i++)
    e += audio[i] * audio[i];

  return e;
}

vector<float>
get_noise_envelope()
{
  WavSet wav_set;
  Error error = wav_set.load ("testnoisesr.tmp.smset");
  assert (!error);

  assert (wav_set.waves.size() == 1);
  Audio *audio = wav_set.waves[0].audio;

  size_t count = 0;
  vector<float> noise_env (32);
  for (size_t pos = 4; pos < audio->contents.size() - 4; pos++)
    {
      AudioBlock *block = &audio->contents[pos];
      assert (noise_env.size() == block->noise.size());
      for (size_t i = 0; i < block->noise.size(); i++)
        noise_env[i] += block->noise_f (i);
      count++;
    }
  for (size_t i = 0; i < noise_env.size(); i++)
    noise_env[i] /= count;
  return noise_env;
}

void
dump_noise_envelope()
{
  vector<float> noise_env = get_noise_envelope();

  for (size_t i = 0; i < noise_env.size(); i++)
    printf ("noise-envelope %zd %.17g\n", i, noise_env[i]);
}

int
reencode (int argc, char **argv)
{
  assert (argc >= 4);
  int from_sr = atoi (argv[2]);
  int to_sr   = atoi (argv[3]);
  string win = (argc >= 5) ? argv[4] : "cos";
  float fundamental_freq = (argc >= 6) ? sm_atof (argv[5]) : 440;

  printf ("# reencode from_sr = %d to_sr = %d, win = %s, fundamental = %f\n", from_sr, to_sr, win.c_str(), fundamental_freq);

  Random random;

  vector<float> noise (from_sr * 5);
  for (size_t i = 0; i < noise.size(); i++)
    noise[i] = random.random_double_range (-0.5, 0.5);

  printf ("noise-from-level %.17g\n", sqrt (energy (noise) / noise.size()));

  encode (noise, from_sr, win, fundamental_freq);
  vector<float> from_env = get_noise_envelope();

  vector<float> audio_out (to_sr * 5);
  decode (audio_out, to_sr);

  printf ("noise-to-level %.17g\n", sqrt (energy (audio_out) / audio_out.size()));

  encode (audio_out, to_sr, win, fundamental_freq);
  vector<float> to_env = get_noise_envelope();

  assert (from_env.size() == to_env.size());
  for (size_t i = 0; i < from_env.size(); i++)
    {
      printf ("noise-envelope %2zd %6.2f %.17g %.17g\n", i, to_env[i] / from_env[i] * 100., from_env[i], to_env[i]);
    }

  delete_tmp_files();
  return 0;
}

int
main (int argc, char **argv)
{
  Random random;

  Main main (&argc, &argv);

// Debug::debug_enable ("encoder");

  if (argc >= 2 && strcmp (argv[1], "reencode") == 0)   // noise reencode test
    {
      return reencode (argc, argv);
    }

  // default noise sr test
  int sr = (argc >= 2) ? atoi (argv[1]) : 48000;
  string win = (argc >= 3) ? argv[2] : "cos";
  float fundamental_freq = (argc >= 4) ? sm_atof (argv[3]) : 440;

  printf ("# sr = %d, win = %s, fundamental_freq = %f\n", sr, win.c_str(), fundamental_freq);

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

  encode (noise, sr, win, fundamental_freq);
  dump_noise_envelope();

  printf ("noise-in-energy %.17g\n", energy (noise_48000));
  printf ("noise-in-level %.17g\n", sqrt (energy (noise_48000) / noise_48000.size()));
  printf ("noise-sr-level %.17g\n", sqrt (energy (noise) / noise.size()));

  vector<float> audio_out (noise_48000.size());
  for (int i = 48000; i < 197000; i += 12000)
    {
      decode_resample (audio_out, i);
      printf ("noise-out-energy %d %.17g\n", i, energy (audio_out));
    }
  delete_tmp_files();
}
