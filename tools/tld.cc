#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"
#include "smmain.hh"

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

using SpectMorph::WavSet;
using SpectMorph::LiveDecoder;
using SpectMorph::sm_init;

using std::vector;
using std::string;

double
gettime ()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
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

  double clocks_per_sec = 2500.0 * 1000 * 1000;

  LiveDecoder decoder (&smset);
  if (argc == 4 && string (argv[3]) == "avg")
    {
      const int n = 20000;
      const int runs = 350;

      vector<float> audio_out (n);
      double start_t = gettime();
      for (int l = 0; l < runs; l++)
        {
          decoder.retrigger (0, freq, 48000);
          decoder.process (n, 0, 0, &audio_out[0]);
        }
      double end_t = gettime();
      printf ("%d %.17g\n", n, (end_t - start_t) * clocks_per_sec / n / runs);
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
              decoder.retrigger (0, freq, 48000);
              decoder.process (n, 0, 0, &audio_out[0]);
            }
          double end_t = gettime();
          printf ("%d %.17g\n", n, (end_t - start_t) * clocks_per_sec / n / runs);
        }
    }
}
