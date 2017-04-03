// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"
#include "smmain.hh"
#include "smutils.hh"
#include "smwavdata.hh"

#include <bse/gsldatahandle.hh>
#include <bse/gsldatautils.hh>

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::min;

double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
adjust_priority()
{
  setpriority (PRIO_PROCESS, getpid(), -20);
  return getpriority (PRIO_PROCESS, getpid());
}

int
main (int argc, char **argv)
{
  /* init */
  sm_init (&argc, &argv);

  assert (argc == 3 || argc == 4);

  WavSet smset;
  if (smset.load (argv[1]) != 0)
    {
      fprintf (stderr, "can't load file %s\n", argv[1]);
      return 1;
    }

  const float freq = atof (argv[2]);
  assert (freq >= 20 && freq < 22000);

  int priority = adjust_priority();
  const double ns_per_sec = 1e9;

  LiveDecoder decoder (&smset);
  decoder.set_noise_seed (42);
  if (argc == 4 && (string (argv[3]) == "avg" || string (argv[3]) == "avg-all"))
    {
      float ns_per_sample[5] = { 0, };
      for (int i = 0; i < 5; i++)
        {
          if (string (argv[3]) != "avg-all" || i == 3)
            {
              bool en = (i & 1);
              bool es = (i & 2);
              decoder.enable_noise (en);
              decoder.enable_sines (es);
              decoder.enable_debug_fft_perf (i == 4);
              const int n = 20000;
              const int runs = 25;

              vector<float> audio_out (n);

              // avoid measuring setup time
              decoder.process (n, 0, 0, &audio_out[0]);
              decoder.retrigger (0, freq, 127, 48000);

              double best_time = 1e7;
              for (int rep = 0; rep < 25; rep++)
                {
                  double start_t = gettime();
                  for (int l = 0; l < runs; l++)
                    {
                      decoder.retrigger (0, freq, 127, 48000);
                      decoder.process (n, 0, 0, &audio_out[0]);
                    }
                  double end_t = gettime();
                  best_time = min (best_time, (end_t - start_t));
                }
              ns_per_sample[i] = best_time * ns_per_sec / n / runs;
            }
        }
      sm_printf ("all..:  %f\n", ns_per_sample[3]);
      sm_printf ("sines:  %f\n", ns_per_sample[2] - ns_per_sample[4]);
      sm_printf ("noise:  %f\n", ns_per_sample[1] - ns_per_sample[4]);
      sm_printf ("fft:    %f\n", ns_per_sample[4] - ns_per_sample[0]);
      sm_printf ("other:  %f\n", ns_per_sample[0]);
      sm_printf ("bogopolyphony = %f\n", ns_per_sec / (ns_per_sample[3] * 48000));
    }
  else if (argc == 4 && (string (argv[3]) == "avg-samples"))
    {
      decoder.enable_original_samples (true);

      const int n = 20000;
      const int runs = 25;

      float audio_out[n];

      // avoid measuring setup time
      decoder.process (n, 0, 0, audio_out);
      decoder.retrigger (0, freq, 127, 48000);

      double best_time = 1e7;
      for (int rep = 0; rep < 25; rep++)
        {
          double start_t = gettime();
          for (int l = 0; l < runs; l++)
            {
              decoder.retrigger (0, freq, 127, 48000);
              decoder.process (n, 0, 0, audio_out);
            }
          double end_t = gettime();
          best_time = min (best_time, (end_t - start_t));
        }
      const double ns_per_sample = best_time * ns_per_sec / n / runs;
      sm_printf ("samples:  %f\n", ns_per_sample);
      sm_printf ("bogopolyphony = %f\n", ns_per_sec / (ns_per_sample * 48000));
    }
  else if (argc == 4 && string (argv[3]) == "export")
    {
      const int SR = 48000;

      vector<float> audio_out (SR * 20);
      decoder.retrigger (0, freq, 127, SR);
      decoder.process (audio_out.size(), 0, 0, &audio_out[0]);

      std::string export_wav = "tld.wav";

      WavData wav_data (audio_out, 1, SR);
      if (!wav_data.save (export_wav))
        {
          sfi_error ("export to file %s failed: %s", export_wav.c_str(), wav_data.error_blurb());
        }
    }
  else
    {
      for (int n = 64; n <= 4096; n += 64)
        {
          vector<float> audio_out (n);
          double start_t = gettime();
          const int runs = 40;
          for (int l = 0; l < runs; l++)
            {
              decoder.retrigger (0, freq, 127, 48000);
              decoder.process (n, 0, 0, &audio_out[0]);
            }
          double end_t = gettime();
          sm_printf ("%d %.17g\n", n, (end_t - start_t) * ns_per_sec / n / runs);
        }
    }
  sm_printf ("# nice priority %d\n", priority);
}
