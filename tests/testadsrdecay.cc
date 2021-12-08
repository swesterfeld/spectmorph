// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smadsrenvelope.hh"
#include "smmain.hh"
#include "smutils.hh"

#include <stdlib.h>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  ADSREnvelope adsr_envelope;

  adsr_envelope.test_decay (atoi (argv[1]), sm_atof (argv[2]), sm_atof (argv[3]));
}
