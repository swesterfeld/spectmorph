// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <math.h>
#include "smmain.hh"
#include "smwavdata.hh"
#include "config.h"

using std::string;
using std::vector;
using namespace SpectMorph;

/// @cond
struct Options
{
  string              program_name  = "ascii2wav";
  int                 rate          = 44100;
  int                 bits          = 16;
  int                 channels      = 1;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options ()
{
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  /*  I am tired of seeing .libs/lt-gst123 all the time,
   *  but basically this should be done (to allow renaming the binary):
   *
  if (argc && argv[0])
    program_name = argv[0];
  */

  for (i = 1; i < argc; i++)
    {
      const char *opt_arg;
      if (strcmp (argv[i], "--help") == 0 ||
          strcmp (argv[i], "-h") == 0)
	{
	  print_usage();
	  exit (0);
	}
      else if (strcmp (argv[i], "--version") == 0 || strcmp (argv[i], "-v") == 0)
	{
	  printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--rate", &opt_arg) || check_arg (argc, argv, &i, "-r", &opt_arg))
	{
	  rate = atoi (opt_arg);
	}
      else if (check_arg (argc, argv, &i, "--bits", &opt_arg) || check_arg (argc, argv, &i, "-b", &opt_arg))
        {
          bits = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--channels", &opt_arg) || check_arg (argc, argv, &i, "-c", &opt_arg))
        {
          channels = atoi (opt_arg);
        }
    }

  /* resort argc/argv */
  e = 1;
  for (i = 1; i < argc; i++)
    if (argv[i])
      {
        argv[e++] = argv[i];
        if (i >= e)
          argv[i] = NULL;
      }
  *argc_p = e;
}

void
Options::print_usage ()
{
  printf ("usage: %s [ <options> ] <output_wav>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" -r, --rate <sampling rate>  set output sample rate manually\n");
  printf (" -b, --bits <bits>           set output bit depth\n");
  printf (" -c, --channels <channels>   set output channels\n");
  printf ("\n");
  printf ("usage example:\n\n");
  printf ("cat waveform.txt | ascii2wav wavform.wav -r 96000\n");
  printf ("\n");
  printf ("set 24 bit export like this:\n\n");
  printf ("cat waveform.txt | ascii2wav wavform.wav -r 96000 -b 24\n");
}

int
main (int argc, char **argv)
{
  /* init */
  Main main (&argc, &argv);
  options.parse (&argc, &argv);

  char buffer[1024];
  vector<float> signal;

  if (argc != 2)
    {
      Options::print_usage();
      return 1;
    }

  string filename = argv[1];
  bool   level_warning = false;

  int line = 1;
  while (fgets (buffer, 1024, stdin) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          char *end_ptr = buffer;
          bool parse_error = false;

          for (int ch = 0; ch < options.channels; ch++)
            {
              char *start_ptr = end_ptr;
              signal.push_back (g_ascii_strtod (start_ptr, &end_ptr));

              if (fabs (signal.back()) > 1.0 && !level_warning)
                {
                  g_printerr ("ascii2wav: warning: input signal level is more than 1.0\n");
                  level_warning = true;
                }

              // check that we parsed at least one character
              parse_error = parse_error || (start_ptr == end_ptr);

              // eat whitespace
              while (*end_ptr == ' ' || *end_ptr == '\n' || *end_ptr == '\t' || *end_ptr == '\r')
                end_ptr++;
            }

          // check that we parsed the whole line
          parse_error = parse_error || (*end_ptr != 0);

          if (parse_error)
            {
              g_printerr ("ascii2wav: parse error on line %d\n", line);
              exit (1);
            }
        }
      line++;
    }
  WavData wav_data (signal, options.channels, options.rate, options.bits);

  if (!wav_data.save (filename))
    {
      g_printerr ("export to file %s failed: %s\n", filename.c_str(), wav_data.error_blurb());
      exit (1);
    }
}
