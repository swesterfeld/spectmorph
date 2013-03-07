// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphlinear.hh"
#include "smmain.hh"
#include "config.h"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::min;

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  int                 midi_note;
  double              len;
  bool                fade;
  bool                quiet;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("smrunplan"),
  midi_note (-1),
  len (1),
  fade (false),
  quiet (false)
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
      else if (check_arg (argc, argv, &i, "--len", &opt_arg) || check_arg (argc, argv, &i, "-l", &opt_arg))
        {
          len = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--fade") || check_arg (argc, argv, &i, "-f"))
        {
          fade = true;
        }
      else if (check_arg (argc, argv, &i, "--quiet") || check_arg (argc, argv, &i, "-q"))
        {
          quiet = true;
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
  printf (" -f, --fade                  fade morphing parameter\n");
  printf (" -l, --len                   set output sample len\n");
  printf (" -m, --midi-note <note>      set midi note to use\n");
  printf (" -q, --quiet                 suppress audio output\n");
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

  MorphPlanSynth synth (44100);
  MorphPlanVoice& voice = *synth.add_voice();
  synth.update_plan (plan);
  assert (voice.output());

  vector<float> samples (44100 * options.len);

  float freq = 440;
  if (options.midi_note >= 0)
    freq = freq_from_note (options.midi_note);

  MorphLinear *linear_op = 0;

  vector<MorphOperator *> ops = plan->operators();
  for (vector<MorphOperator *>::iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *op = *oi;
      g_printerr ("  Operator: %s (%s)\n", op->name().c_str(), op->type_name().c_str());
      if (op->type_name() == "Linear")
        linear_op = dynamic_cast<MorphLinear *> (op);
    }

  voice.output()->retrigger (0, freq, 100);

  const size_t STEP = 100;
  for (size_t i = 0; i < samples.size(); i += STEP)
    {
      if (options.fade)
        {
          double morphing = 2 * double (i) / samples.size() - 1;
          linear_op->set_morphing (morphing);
          synth.update_plan (plan);
        }

      size_t todo = min (STEP, samples.size());

      float *audio_out[1] = { &samples[i] };
      voice.output()->process (todo, audio_out, 1);

      synth.update_shared_state (todo * 1000.0 / voice.mix_freq());
    }
  if (!options.quiet)
    {
      for (size_t i = 0; i < samples.size(); i++)
        {
          printf ("%.17g\n", samples[i]);
        }
    }
}
