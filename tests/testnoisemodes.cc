// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smnoisedecoder.hh"
#include "smifftsynth.hh"
#include "smmain.hh"
#include "smrandom.hh"
#include "smfft.hh"

#include <assert.h>

#define ASSERT_PRINTF(cond, fmt, ...) do { \
    auto test_output = string_printf (fmt "\n", ##__VA_ARGS__); \
    if (!(cond)) { \
        fprintf(stderr, "Assertion failed: %s\n", #cond); \
        fprintf(stderr, "  %s", test_output.c_str()); \
        fprintf(stderr, "  File: %s, Line: %d\n", __FILE__, __LINE__); \
        abort(); \
    }  \
    else { \
      printf("%s", test_output.c_str()); \
    } \
  } while (0)

using namespace SpectMorph;
using std::max;
using std::vector;

void
swap_half_blocks (float *samples, size_t block_size)
{
  float out_samples[block_size];
  for (size_t i = 0; i < block_size / 2; i++)
    {
      out_samples[i] = samples[i + block_size / 2];
      out_samples[i + block_size / 2] = samples[i];
    }
  std::copy_n (out_samples, block_size, samples);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  FFT::debug_in_test_program (true);

  double mix_freq = 48000;
  size_t block_size = 1024;

  AudioBlock audio_block;
  NoiseDecoder noise_dec (mix_freq, block_size);
  IFFTSynth ifft_synth (block_size, mix_freq, IFFTSynth::WIN_HANN);
  Random    random;

  float *samples = FFT::new_array_float (block_size);
  float *win_samples = FFT::new_array_float (block_size);
  float *cos_win_samples = FFT::new_array_float (block_size);
  float *xwin_samples = FFT::new_array_float (block_size);
  float *fft_samples = FFT::new_array_float (block_size);

  const double white_noise_level = 0.0025; // constant to keep output approximately in range [-1;1]
  random.set_seed (42);
  for (int i = 0; i < 32; i++)
    audio_block.noise.push_back (sm_factor2idb (white_noise_level * random.random_double_range (0.5, 1.0)));

  // NOISE DEBUG MODE
  noise_dec.set_seed (42);
  noise_dec.process (audio_block.noise.data(), samples, NoiseDecoder::DEBUG_UNWINDOWED);
  for (size_t i = 0; i < block_size / 2; i++)
    {
      win_samples[i] = samples[i + block_size / 2];
      win_samples[i + block_size / 2] = samples[i];
    }
  for (size_t i = 0; i < block_size; i++)
    {
      cos_win_samples[i] = window_cos (2.0 * i / block_size - 1.0) * win_samples[i];
      win_samples[i] *= window_blackman_harris_92 (2.0 * i / block_size - 1.0);
    }
  for (size_t i = 0; i < block_size / 2; i++)
    {
      xwin_samples[i] = win_samples[i + block_size / 2];
      xwin_samples[i + block_size / 2] = win_samples[i];
    }
  float *dbg_spectrum = FFT::new_array_float (block_size);
  FFT::fftar_float (block_size, xwin_samples, dbg_spectrum);

  // Check IFFT NOISE with hanning window
  noise_dec.set_seed (42);
  noise_dec.process (audio_block.noise.data(), ifft_synth.fft_input(), NoiseDecoder::SET_SPECTRUM_HANN);
  zero_float_block (block_size, fft_samples);
  FFT::fftsr_destructive_float (block_size, ifft_synth.fft_input(), fft_samples);
  swap_half_blocks (fft_samples, block_size);
  float hann_diff_max = 0;
  for (size_t i = 0; i < block_size; i++)
    {
      hann_diff_max = max (hann_diff_max, fabs (cos_win_samples[i] - fft_samples[i]));
#if 0
      sm_printf ("%f #W\n", cos_win_samples[i]);
      sm_printf ("%f #H\n", fft_samples[i]);
#endif
    }
  ASSERT_PRINTF (hann_diff_max < 2e-7, "noise test: hann_diff_max=%.17g", hann_diff_max);

  // NOISE IFFT MODE: BH92 spectrum
  for (int sse = 0; sse < 2; sse++)
    {
      sm_enable_sse (sse);
      ifft_synth.clear_partials();
      noise_dec.set_seed (42);
      noise_dec.process (audio_block.noise.data(), ifft_synth.fft_input(), NoiseDecoder::ADD_SPECTRUM_BH92);

      vector<float> spectrum (ifft_synth.fft_input(), ifft_synth.fft_input() + block_size);

      ifft_synth.get_samples (fft_samples);

#if 0
      for (int i = 0; i < block_size; i++)
        printf ("T %d %.17g %.17g\n", i, cos_win_samples[i], fft_samples[i]);

      for (int i = 0; i < block_size; i++)
        printf ("F %d %.17g %.17g\n", i, ifft_synth.fft_buffer()[i], dbg_spectrum[i] / block_size);
#endif
      float t_diff_max = 0;
      for (size_t i = 0; i < block_size; i++)
        t_diff_max = max (t_diff_max, fabs (cos_win_samples[i] - fft_samples[i]));
      float s_diff_max = 0;
      for (size_t i = 0; i < block_size; i++)
        s_diff_max = max (s_diff_max, fabs (spectrum[i] - dbg_spectrum[i] / block_size));
      ASSERT_PRINTF (t_diff_max < 2e-6, "noise test: sse=%d t_diff_max=%.17g", sse, t_diff_max);
      ASSERT_PRINTF (s_diff_max < 3e-9, "noise test: sse=%d s_diff_max=%.17g", sse, s_diff_max);
    }

  FFT::free_array_float (samples);
  FFT::free_array_float (win_samples);
  FFT::free_array_float (cos_win_samples);
  FFT::free_array_float (xwin_samples);
  FFT::free_array_float (fft_samples);
  FFT::free_array_float (dbg_spectrum);
}
