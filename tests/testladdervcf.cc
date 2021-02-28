// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <vector>
#include <string>

#include <cmath>
#include <cstdio>

#include "smladdervcf.hh"

using std::vector;
using std::string;

using namespace SpectMorph;

void
gen_sweep (vector<float>& left, vector<float>& right, vector<float>& freq)
{
  double phase = 0;
  double l = 48000 * 5;
  double factor = pow (24000 / 20., (1./l));
  double vol = 0;
  for (double f = 20; f < 24000; f *= factor)
    {
      freq.push_back (f);
      left.push_back (sin (phase) * vol);
      right.push_back (cos (phase) * vol);
      phase += f / 48000 * 2 * M_PI;
      vol += 1. / 500; /* avoid click at start */
      if (vol > 1)
        vol = 1;
    }
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      printf ("too few args\n");
      return 1;
    }

  string cmd = argv[1];
  if (argc == 4 && cmd == "sweep")
    {
      float cutoff = atof (argv[2]);
      float resonance = atof (argv[3]);
      vector<float> left;
      vector<float> right;
      vector<float> freq;
      gen_sweep (left, right, freq);
      LadderVCFLinear laddervcf;

      for (size_t i = 0; i < left.size(); i++)
        {
          const float *inputs[2]  = { &left[i], &right[i] };
          float       *outputs[2] = { &left[i], &right[i] };

          laddervcf.run_block (1, cutoff / 24000., resonance, inputs, outputs, true, true, nullptr, nullptr);
        }

      for (size_t i = 0; i < left.size(); i++)
        printf ("%f %.17g\n", freq[i], sqrt (left[i] * left[i] + right[i] * right[i]));

      return 0;
    }
}
