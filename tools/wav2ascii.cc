// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "smmain.hh"
#include "smutils.hh"
#include "smwavdata.hh"

using std::string;
using std::vector;

using namespace SpectMorph;

/// @cond
struct Options
{
  string program_name;
} options;
/// @endcond

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  options.program_name = "wav2ascii";

  if (argc != 2)
    {
      printf ("usage: %s audiofile.wav\n", options.program_name.c_str());
      exit (1);
    }

  /* open input */
  WavData wav_data;
  if (!wav_data.load (argv[1]))
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], wav_data.error_blurb());
      exit (1);
    }

  uint n_channels = wav_data.n_channels();
  if (n_channels != 1 && n_channels != 2)
    {
      fprintf (stderr, "%s: input file %s: only mono or stereo files are supported\n", options.program_name.c_str(), argv[1]);
      exit (1);
    }
  for (size_t pos = 0; pos < wav_data.n_values(); pos += n_channels)
    {
      if (n_channels == 1)
        sm_printf ("%.17g\n", wav_data[pos]);
      else
        sm_printf ("%.17g %.17g\n", wav_data[pos], wav_data[pos + 1]);
    }
}
