// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <bse/bseloader.hh>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "smmain.hh"
#include "smutils.hh"
#include "smwavdata.hh"

using std::string;
using std::vector;

using SpectMorph::sm_init;
using SpectMorph::sm_printf;
using SpectMorph::WavData;
using SpectMorph::Error;

/// @cond
struct Options
{
  string program_name;
} options;
/// @endcond

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  options.program_name = "wav2ascii";

  if (argc != 2)
    {
      printf ("usage: %s audiofile.wav\n", options.program_name.c_str());
      exit (1);
    }

  /* open input */
  WavData wav_data;
  Error error = wav_data.load (argv[1]);
  if (error != 0)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], sm_error_blurb (error));
      exit (1);
    }

  if (wav_data.n_channels() != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }

  for (size_t pos = 0; pos < wav_data.n_values(); pos++)
    {
      sm_printf ("%.17g\n", wav_data[pos]);
    }
}
