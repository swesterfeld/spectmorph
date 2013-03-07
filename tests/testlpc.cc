// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlpc.hh"
#include "smmain.hh"
#include "smwavloader.hh"
#include <assert.h>
#include <math.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  assert (argc == 2);

  WavLoader *wav_loader = WavLoader::load (argv[1]);
  assert (wav_loader);

  const vector<float>& samples = wav_loader->samples();

  vector<double> lpc (50);
  vector<float> lsf_p, lsf_q;

  LPC::compute_lpc (lpc, &samples[0], &samples[samples.size()]);
  LPC::lpc2lsf (lpc, lsf_p, lsf_q);
  assert (lsf_p.size() == lsf_q.size());

  LPC::LSFEnvelope env;
  env.init (lsf_p, lsf_q);
  for (double freq = 0; freq < M_PI; freq += 0.001)
    {
      double lpc_value = LPC::eval_lpc (lpc, freq);
      double value = env.eval (freq);
      printf ("%f %.17g %.17g\n", freq / (2 * M_PI) * 44100, value, lpc_value);
    }
  delete wav_loader;
}
