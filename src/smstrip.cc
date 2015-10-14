// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>

#include "smaudio.hh"
#include "smmain.hh"
#include "config.h"

using std::string;
using SpectMorph::sm_init;

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  bool                keep_samples;
  bool                strip_lpc;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("smstrip"),
  keep_samples (false),
  strip_lpc (false)
{
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  g_return_if_fail (argc >= 0);

  /*  I am tired of seeing .libs/lt-gst123 all the time,
   *  but basically this should be done (to allow renaming the binary):
   *
  if (argc && argv[0])
    program_name = argv[0];
  */

  for (i = 1; i < argc; i++)
    {
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
      else if (check_arg (argc, argv, &i, "--strip-lpc") || check_arg (argc, argv, &i, "-l"))
	{
          strip_lpc = true;
	}
      else if (check_arg (argc, argv, &i, "--keep-samples"))
        {
          keep_samples = true;
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
  printf ("usage: %s [ <options> ] <sm_file> [...]\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" --keep-samples              keep original samples\n");
  printf (" -l, --strip-lpc             strip lpc data\n");
  printf ("\n");
}


int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);

  if (argc < 2)
    {
      printf ("usage: smstrip <filename.sm> [...]\n");
      exit (1);
    }
  for (int n = 1; n < argc; n++)
    {
      SpectMorph::Audio audio;
      Bse::ErrorType error = audio.load (argv[n], SpectMorph::AUDIO_SKIP_DEBUG);
      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[n], bse_error_blurb (error));
          exit (1);
        }
      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          if (options.strip_lpc)
            {
              audio.contents[i].lpc_lsf_p.clear();
              audio.contents[i].lpc_lsf_q.clear();
            }
          audio.contents[i].debug_samples.clear();
          audio.contents[i].original_fft.clear();
        }
      if (!options.keep_samples)
        {
          audio.original_samples.clear();
        }
      audio.save (argv[n]);
    }
}
