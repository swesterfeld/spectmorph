// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"
#include "smmain.hh"

#include <bse/gsldatahandle.h>
#include <bse/gsldatautils.h>

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

using SpectMorph::WavSet;
using SpectMorph::LiveDecoder;
using SpectMorph::sm_init;

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
  if (smset.load (argv[1]))
    {
      fprintf (stderr, "can't load file %s\n", argv[1]);
      return 1;
    }

  const float freq = atof (argv[2]);
  assert (freq >= 20 && freq < 22000);

  int priority = adjust_priority();
  double clocks_per_sec = 2500.0 * 1000 * 1000;

  LiveDecoder decoder (&smset);
  if (argc == 4 && (string (argv[3]) == "avg" || string (argv[3]) == "avg-all"))
    {
      float clocks_per_sample[5] = { 0, };
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
              clocks_per_sample[i] = best_time * clocks_per_sec / n / runs;
            }
        }
      printf ("all..:  %f\n", clocks_per_sample[3]);
      printf ("sines:  %f\n", clocks_per_sample[2] - clocks_per_sample[4]);
      printf ("noise:  %f\n", clocks_per_sample[1] - clocks_per_sample[4]);
      printf ("fft:    %f\n", clocks_per_sample[4] - clocks_per_sample[0]);
      printf ("other:  %f\n", clocks_per_sample[0]);
      printf ("bogopolyphony = %f\n", clocks_per_sec / (clocks_per_sample[3] * 48000));
    }
  else if (argc == 4 && (string (argv[3]) == "avg-samples"))
    {
      float clocks_per_sample;
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
      clocks_per_sample = best_time * clocks_per_sec / n / runs;
      printf ("samples:  %f\n", clocks_per_sample);
      printf ("bogopolyphony = %f\n", clocks_per_sec / (clocks_per_sample * 48000));
    }
  else if (argc == 4 && string (argv[3]) == "export")
    {
      const int SR = 48000;

      vector<float> audio_out (SR * 20);
      decoder.retrigger (0, freq, 127, SR);
      decoder.process (audio_out.size(), 0, 0, &audio_out[0]);

      GslDataHandle *out_dhandle = gsl_data_handle_new_mem (1, 32, SR, SR / 16 * 2048, audio_out.size(), &audio_out[0], NULL);
      BseErrorType error = gsl_data_handle_open (out_dhandle);
      if (error)
        {
          fprintf (stderr, "can not open mem dhandle for exporting wave file\n");
          exit (1);
        }

      std::string export_wav = "tld.wav";
      int fd = open (export_wav.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd < 0)
        {
          BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
          sfi_error ("export to file %s failed: %s", export_wav.c_str(), bse_error_blurb (error));
        }
      int xerrno = gsl_data_handle_dump_wav (out_dhandle, fd, 16, out_dhandle->setup.n_channels, (guint) out_dhandle->setup.mix_freq);
      if (xerrno)
        {
          BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
          sfi_error ("export to file %s failed: %s", export_wav.c_str(), bse_error_blurb (error));
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
          printf ("%d %.17g\n", n, (end_t - start_t) * clocks_per_sec / n / runs);
        }
    }
  printf ("# nice priority %d\n", priority);
}
