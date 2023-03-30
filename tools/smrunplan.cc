// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphlinear.hh"
#include "smmorphgrid.hh"
#include "smmain.hh"
#include "smutils.hh"
#include "smsynthinterface.hh"
#include "config.h"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::min;
using std::max;

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  int                 midi_note;
  double              len;
  bool                fade;
  vector<double>      fade_env;
  bool                quiet;
  bool                perf;
  bool                normalize;
  double              gain;
  int                 rate;

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
  quiet (false),
  normalize (false),
  gain (1.0),
  rate (44100)
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
      else if (check_arg (argc, argv, &i, "--midi-note", &opt_arg) || check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          midi_note = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--len", &opt_arg) || check_arg (argc, argv, &i, "-l", &opt_arg))
        {
          len = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--fade") || check_arg (argc, argv, &i, "-f"))
        {
          fade = true;
        }
      else if (check_arg (argc, argv, &i, "--fade-env", &opt_arg) ||
               check_arg (argc, argv, &i, "-e", &opt_arg))
        {
          fade = true;
          string field;

          while (*opt_arg)  // parse envelope arg: "<point1>,<point2>,...,<pointN>"
            {
              if (*opt_arg == ',')
                {
                  fade_env.push_back (sm_atof (field.c_str()));
                  field = "";
                }
              else
                {
                  field += *opt_arg;
                }
              opt_arg++;
            }
          fade_env.push_back (sm_atof (field.c_str()));
        }
      else if (check_arg (argc, argv, &i, "--quiet") || check_arg (argc, argv, &i, "-q"))
        {
          quiet = true;
        }
      else if (check_arg (argc, argv, &i, "--normalize") || check_arg (argc, argv, &i, "-n"))
        {
          normalize = true;
        }
      else if (check_arg (argc, argv, &i, "--perf") || check_arg (argc, argv, &i, "-p"))
        {
          perf = true;
        }
      else if (check_arg (argc, argv, &i, "--gain", &opt_arg) || check_arg (argc, argv, &i, "-g", &opt_arg))
        {
          gain = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--rate", &opt_arg) || check_arg (argc, argv, &i, "-r", &opt_arg))
        {
          rate = atoi (opt_arg);
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
  printf (" -e, --fade-env <envelope>   fade morphing according to envelope\n");
  printf (" -n, --normalize             normalize output samples\n");
  printf (" -p, --perf                  run performance test\n");
  printf (" -l, --len <len>             set output sample len\n");
  printf (" -m, --midi-note <note>      set midi note to use\n");
  printf (" -q, --quiet                 suppress audio output\n");
  printf ("\n");
}

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

class Player
{
  MorphLinear    *linear_op;
  MorphGrid      *grid_op;
  MorphPlanVoice *voice;

  Project         project;
  MorphPlan      *plan;
  MorphPlanSynth  synth;
public:
  Player();

  void load_plan (const string& filename);
  void retrigger();
  void compute_samples (vector<float>& samples);
};

Player::Player() :
  linear_op (0),
  grid_op (0),
  voice (0),
  plan (project.morph_plan()),
  synth (options.rate, 1)
{
}

void
Player::load_plan (const string& filename)
{
  project.set_mix_freq (options.rate);

  Error error = project.load (filename);
  if (error)
    {
      fprintf (stderr, "%s: loading file '%s' failed: %s\n", options.program_name.c_str(), filename.c_str(), error.message());
      exit (1);
    }

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());

  voice = synth.voice (0);
  auto update = synth.prepare_update (*plan);
  synth.apply_update (update);
  assert (voice->output());

  /* search operators for --fade, --fade-env */
  vector<MorphOperator *> ops = plan->operators();
  for (vector<MorphOperator *>::iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *op = *oi;
      g_printerr ("  Operator: %s (%s)\n", op->name().c_str(), op->type_name().c_str());
      if (op->type_name() == "Linear")
        linear_op = dynamic_cast<MorphLinear *> (op);
      else if (op->type_name() == "Grid")
        grid_op = dynamic_cast<MorphGrid *> (op);
    }
}

void
Player::retrigger()
{
  float freq = 440;
  if (options.midi_note >= 0)
    freq = freq_from_note (options.midi_note);

  voice->output()->retrigger (/* zero time */ TimeInfo(), 0, freq, 100);
}

void
Player::compute_samples (vector<float>& samples)
{
  uint64 audio_time_stamp = 0;
  const size_t STEP = 100;
  for (size_t i = 0; i < samples.size(); i += STEP)
    {
      if (options.fade)
        {
          double morphing;
          const int env_len = options.fade_env.size();
          if (env_len)
            {
              const double fpos = double (i) / samples.size() * (env_len - 1);
              const int    left = sm_bound<int> (0, fpos, env_len - 1);
              const int    right = sm_bound (0, left + 1, env_len - 1);
              const double interp = fpos - left;
              morphing = 2 * (options.fade_env[left] * (1 - interp) + options.fade_env[right] * interp) - 1;
            }
          else
            {
              morphing = 2 * double (i) / samples.size() - 1;
            }
          if (linear_op)
            {
              linear_op->set_morphing (morphing);
            }
          else if (grid_op)
            {
              grid_op->set_x_morphing (morphing);
            }
          else
            {
              g_assert_not_reached();
            }
          auto update = synth.prepare_update (*plan);
          synth.apply_update (update);
        }

      size_t todo = min (STEP, samples.size() - i);

      double ppq_pos = 0; // FIXME
      TimeInfoGenerator time_info_gen;

      time_info_gen.start_block (audio_time_stamp * 1000.0 / voice->mix_freq(), ppq_pos);

      synth.update_shared_state (time_info_gen.time_info (0));

      float *audio_out[1] = { &samples[i] };
      voice->output()->process (time_info_gen, todo, audio_out, 1);

      audio_time_stamp += todo;
    }
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  options.parse (&argc, &argv);

  if (argc != 2)
    {
      printf ("usage: %s <plan>\n", argv[0]);
      exit (1);
    }

  vector<float> samples (options.rate * options.len);
  Player        player;

  player.load_plan (argv[1]);
  player.retrigger();

  if (options.perf)
    {
      player.compute_samples (samples); // warmup

      double start = get_time();

      // at 100 bogo-voices, test should run 10 seconds
      const size_t RUNS = max<int> (1, options.rate * 1000 / samples.size());
      for (size_t i = 0; i < RUNS; i++)
        player.compute_samples (samples);

      double end = get_time();

      const double ns_per_sec = 1e9;
      sm_printf ("%6.2f ns/sample\n", (end - start) * ns_per_sec / (RUNS * samples.size()));
      sm_printf ("%6.2f bogo-voices\n", double (RUNS * samples.size()) / options.rate / (end - start));

      return 0;
    }
  else
    {
      player.compute_samples (samples);
    }

  // apply gain
  for (size_t i = 0; i < samples.size(); i++)
    samples[i] *= options.gain;

  if (options.normalize)
    {
      float max_peak = 0.0001;
      for (size_t i = 0; i < samples.size(); i++)
        max_peak = max (max_peak, fabs (samples[i]));

      const double factor = 1 / max_peak;
      for (size_t i = 0; i < samples.size(); i++)
        samples[i] *= factor;
    }
  if (!options.quiet)
    {
      for (size_t i = 0; i < samples.size(); i++)
        sm_printf ("%.17g\n", samples[i]);
    }
}
