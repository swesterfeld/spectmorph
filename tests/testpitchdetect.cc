// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smpitchdetect.hh"

#include <cassert>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  assert (argc == 2);

  WavData wav_data;
  bool ok = wav_data.load (argv[1]);
  assert (ok);

  sm_printf ("%.2f\n", detect_pitch (wav_data));
}
