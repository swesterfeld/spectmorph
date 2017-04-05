// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnoisedecoder.hh"
#include "smifftsynth.hh"
#include "smmain.hh"
#include "smrandom.hh"
#include "smfft.hh"

#include <assert.h>
#include <sys/time.h>

using namespace SpectMorph;
using std::max;
using std::min;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  const double mix_freq = 48000;
  const size_t block_size = 1024;

  AudioBlock audio_block;
  NoiseDecoder noise_dec (mix_freq, mix_freq, block_size);
  IFFTSynth ifft_synth (block_size, mix_freq, IFFTSynth::WIN_HANNING);
  Random    random;

  random.set_seed (42);
  for (int i = 0; i < 32; i++)
    audio_block.noise.push_back (sm_factor2idb (random.random_double_range (0.1, 1.0)));

  const int RUNS = 20000, REPS = 13;

  vector<float> samples (block_size);
  double min_time[4] = { 1e20, 1e20, 1e20, 1e20 };
  for (int mode = 0; mode < 4; mode++)
    {
      int ifft = (mode == 0) ? 1 : 0;
      int spect = (mode < 3) ? 1 : 0;
      int sse = (mode == 2) ? 0 : 1;
      sm_enable_sse (sse);
      for (int reps = 0; reps < REPS; reps++)
        {
          double start = gettime();
          for (int r = 0; r < RUNS; r++)
            {
              ifft_synth.clear_partials();
              noise_dec.process (audio_block, ifft_synth.fft_buffer(), spect ? NoiseDecoder::FFT_SPECTRUM : NoiseDecoder::DEBUG_NO_OUTPUT);
              if (ifft)
                ifft_synth.get_samples (&samples[0]);
            }
          double end = gettime();
          min_time[mode] = min (min_time[mode], end - start);
        }
    }
  const double ns_per_sec = 1e9;
  /* we use overlap-add sythesis - for each output sample two noise blocks are used
   * so we need to scale our times with a factor of 2 to get per-output-sample costs
   */
  const double time_norm = 2 * ns_per_sec / RUNS / block_size;
  printf ("noise decoder (spectrum gen): %2f ns/sample\n", min_time[3] * time_norm);
  printf ("noise decoder (convolve):     %2f ns/sample\n", (min_time[2] - min_time[3]) * time_norm);
  printf ("noise decoder (convolve/SSE): %2f ns/sample\n", (min_time[1] - min_time[3]) * time_norm);
  printf ("noise decoder (ifft):         %2f ns/sample\n", (min_time[0] - min_time[1]) * time_norm);
}
