// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include "smmath.hh"
#include "smmain.hh"

#include <vector>

using namespace SpectMorph;
using std::vector;

void
run (ADSREnvelope& adsr_envelope, size_t samples)
{
  vector<float> input (samples, 1.0);
  adsr_envelope.process (input.size(), &input[0]);
  for (auto v : input)
    printf ("%f\n", v);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  float rate = 48000;

  ADSREnvelope adsr_envelope;

  adsr_envelope.set_config (atof (argv[1]), atof (argv[2]), atof (argv[3]), atof (argv[4]), rate);
  adsr_envelope.retrigger();
  run (adsr_envelope, sm_round_positive (rate / 2));
  adsr_envelope.release();
  run (adsr_envelope, sm_round_positive (rate / 2));
}
