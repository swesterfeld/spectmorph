/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"
#include "config.h"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  int                 midi_note;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("smrunplan"),
  midi_note (-1)
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
      else if (check_arg (argc, argv, &i, "--midi-note", &opt_arg) || check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          midi_note = atoi (opt_arg);
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
  printf ("usage: %s [ <options> ] <sm_file>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" -m, --midi-note <note>      set midi note to use\n");
  printf ("\n");
}

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);
  if (argc != 2)
    {
      printf ("usage: %s <plan>\n", argv[0]);
      exit (1);
    }

  MorphPlanPtr plan = new MorphPlan();
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan->load (in);
  delete in;

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());

  MorphPlanVoice voice (plan);
  assert (voice.output());

  vector<float> samples (44100);

  float freq = 440;
  if (options.midi_note >= 0)
    freq = freq_from_note (options.midi_note);

  voice.output()->retrigger (0, freq, 100, 44100);
  voice.output()->process (/* port */ 0, samples.size(), &samples[0]);
  for (size_t i = 0; i < samples.size(); i++)
    {
      printf ("%.17g\n", samples[i]);
    }
}
