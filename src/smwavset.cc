#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include <smwavset.hh>

#include <string>

using std::string;

/// @cond
struct Options
{
  string	program_name; /* FIXME: what to do with that */
  enum { NONE, INIT, ADD } mode;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

Options::Options ()
{
  program_name = "smwavset";
  mode = NONE;
}

#include "stwutils.hh"

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
      else if (check_arg (argc, argv, &i, "-i") || check_arg (argc, argv, &i, "--init"))
        {
          mode = INIT;
        }
      else if (check_arg (argc, argv, &i, "-a") || check_arg (argc, argv, &i, "--add"))
        {
          mode = ADD;
        }
#if 0
      else if (check_arg (argc, argv, &i, "-d"))
	{
          debug = fopen ("/tmp/stwenc.log", "w");
	}
      else if (check_arg (argc, argv, &i, "-f", &opt_arg))
	{
	  fundamental_freq = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          fundamental_freq = freqFromNote (atoi (opt_arg));
        }
      else if (check_arg (argc, argv, &i, "-O0"))
        {
          optimization_level = 0;
        }
      else if (check_arg (argc, argv, &i, "-O1"))
        {
          optimization_level = 1;
        }
      else if (check_arg (argc, argv, &i, "-O2"))
        {
          optimization_level = 2;
        }
      else if (check_arg (argc, argv, &i, "-O", &opt_arg))
        {
          optimization_level = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-s"))
        {
          strip_models = true;
        }
#endif
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
  g_printerr ("usage: %s [ <options> ] <src_audio_file> <dest_sm_file>\n", options.program_name.c_str());
  g_printerr ("\n");
  g_printerr ("options:\n");
  g_printerr (" -h, --help                  help for %s\n", options.program_name.c_str());
  g_printerr (" --version                   print version\n");
  g_printerr (" -f <freq>                   specify fundamental frequency in Hz\n");
  g_printerr (" -m <note>                   specify midi note for fundamental frequency\n");
  g_printerr (" -O <level>                  set optimization level\n");
  g_printerr (" -s                          produced stripped models\n");
  g_printerr ("\n");
}


int
main (int argc, char **argv)
{
  options.parse (&argc, &argv);

  if (options.mode == Options::INIT)
    {
      for (int i = 1; i < argc; i++)
        {
          SpectMorph::WavSet wset;
          wset.save (argv[i]);
        }
    }
  else if (options.mode == Options::ADD)
    {
      assert (argc == 4);

      SpectMorph::WavSet wset;
      wset.load (argv[1]);
      SpectMorph::WavSetWave new_wave;
      new_wave.midi_note = atoi (argv[2]);
      new_wave.path = argv[3];
      wset.waves.push_back (new_wave);
      wset.save (argv[1]);
    }
  else
    {
      g_printerr ("You need to specify a mode (-i)\n");
      exit (1);
    }
}
