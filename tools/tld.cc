#include <bse/bsemain.h>

#include "smwavset.hh"
#include "smlivedecoder.hh"

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

using SpectMorph::WavSet;
using SpectMorph::LiveDecoder;
using std::vector;

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
  SfiInitValue values[] = {
    { "stand-alone",            "true" }, /* no rcfiles etc. */
    { "wave-chunk-padding",     NULL, 1, },
    { "dcache-block-size",      NULL, 8192, },
    { "dcache-cache-memory",    NULL, 5 * 1024 * 1024, },
    { NULL }
  };
  bse_init_inprocess (&argc, &argv, NULL, values);

  assert (argc == 2);

  WavSet smset;
  if (smset.load (argv[1]))
    {
      fprintf (stderr, "can't load file %s\n", argv[1]);
      return 1;
    }

  LiveDecoder decoder (&smset);
  for (int n = 64; n <= 4096; n += 64)
    {
      vector<float> audio_out (n);
      double start_t = gettime();
      for (int l = 0; l < 40; l++)
        {
          decoder.retrigger (50, 48000);
          decoder.process (n, 0, 0, &audio_out[0]);
        }
      double end_t = gettime();
      printf ("%d %.17g\n", n, end_t - start_t);
    }
}
