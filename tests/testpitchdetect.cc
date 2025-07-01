// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smpitchdetect.hh"

#include <cassert>

using namespace SpectMorph;

using std::vector;
using std::pair;

static const auto trumpet_60_expect = std::make_pair (
  263.08582059999997682, -0.12743576281645577
);
static const vector<double> trumpet_60 =
{
  263.277580, 0.006140,
  526.240583, 0.025908,
  789.601832, 0.043100,
  1052.645563, 0.067725,
  1315.429103, 0.079102,
  1578.567167, 0.060438,
  1841.486407, 0.039342,
  2104.614717, 0.029604,
  2367.487276, 0.024651,
  2630.581131, 0.018356,
  2893.234587, 0.016206,
  3155.788129, 0.013738,
  3419.095249, 0.011293,
  3682.147715, 0.008978,
  3944.480632, 0.006035,
  4208.487750, 0.004322,
  4471.345932, 0.004356,
  4733.693274, 0.003283,
  4995.997046, 0.003242,
  5259.474035, 0.002342,
  5521.825605, 0.002278,
  5785.461377, 0.001688,
  6048.123909, 0.001569,
  6312.146690, 0.001222,
  6574.878995, 0.001101,
  6838.365457, 0.000836,
};

static const auto sven_ih_42_expect = std::make_pair (
  91.218589893051316, 2.1525047085249298
);
static const vector<double> sven_ih_42 =
{
//  91.190898, 0.067204,  (( test missing fundamental ))
  182.121314, 0.049994,
  274.732373, 0.029557,
  365.570022, 0.011106,
  453.768492, 0.003825,
  548.967881, 0.002407,
  639.709515, 0.000837,
  1551.824181, 0.000676,
  1642.792664, 0.000866,
  1732.741404, 0.001225,
  1828.203253, 0.002926,
  1924.199117, 0.005959,
  2000.922666, 0.001930,
  2099.709257, 0.001432,
  2189.249508, 0.000986,
  2278.925777, 0.000803,
  2370.582352, 0.000744,
  2463.472600, 0.001219,
  2555.507713, 0.001335,
  2647.720219, 0.002707,
  2740.921542, 0.004018,
  2838.178412, 0.011775,
  2918.140575, 0.010286,
  3014.056793, 0.006544,
  3103.770096, 0.006364,
  3202.154497, 0.007962,
  3289.032220, 0.011444,
  3375.942982, 0.007813,
  3468.363204, 0.007161,
  3558.529745, 0.005409,
  3651.736596, 0.005300,
  3741.304624, 0.004989,
  3828.393553, 0.001710,
  4108.073098, 0.000706,
  4315.995426, 0.000684,
  4416.706222, 0.001345,
  4470.085634, 0.001252,
  5247.269657, 0.000956,
};

void
test (const char *name, const vector<double>& freqs_mags, const pair<double, double> expect)
{
  auto [ freq, error ] = pitch_detect_twm_test (freqs_mags);
  auto [ xfreq, xerror ] = expect;
  double f_delta = std::abs (freq - xfreq);
  double e_delta = std::abs (error - xerror);
  sm_printf ("%s: %.17g %.17g (f_delta=%.17g e_delta=%.17g)\n", name, freq, error, f_delta, e_delta);
  assert (f_delta < 1e-7);
  assert (e_delta < 1e-7);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  if (argc == 2)
    {
      WavData wav_data;
      bool ok = wav_data.load (argv[1]);
      assert (ok);

      sm_printf ("%.2f\n", detect_pitch (wav_data));
    }
  else
    {
      test ("trumpet_60", trumpet_60, trumpet_60_expect);
      test ("sven_ih_42", sven_ih_42, sven_ih_42_expect);
    }
}
