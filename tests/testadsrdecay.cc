// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include "smmain.hh"
#include "smmath.hh"

#include <stdlib.h>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  ADSREnvelope adsr_envelope;

  adsr_envelope.test_decay (atoi (argv[1]), sm_atof (argv[2]), sm_atof (argv[3]));
}
