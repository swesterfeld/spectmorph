// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaudio.hh"
#include "smmain.hh"
#include "smlpc.hh"

#include <assert.h>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  assert (argc == 3);

  Audio audio;

  audio.load (argv[1]);

  const AudioBlock& block = audio.contents[atoi (argv[2])];

  LPC::LSFEnvelope env;
  env.init (block.lpc_lsf_p, block.lpc_lsf_q);

  for (size_t i = 0; i < 1024; i++)
    {
      double f = i / 1024. * M_PI;
      printf ("%f %.17g\n", f, env.eval (f));
    }
}
