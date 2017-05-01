// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavset.hh"
#include "smmain.hh"
#include "smutils.hh"
#include "smlivedecoder.hh"
#include "smwavdata.hh"
#include "config.h"

#include <ao/ao.h>
#include <glib.h>
#include <string.h>
#include <fcntl.h>

#include <string>
#include <vector>

using namespace SpectMorph;

using std::string;
using std::vector;

/// @cond
struct Options
{
  string      program_name; /* FIXME: what to do with that */
  string      export_wav;
  float       freq;
  string      freq_in;
  bool        enable_original_samples;
  bool        text;
  double      gain;
  double      loop;
  int         rate;
  bool        deterministic_random;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

Options::Options () :
  program_name ("smlive"),
  freq (-1),
  enable_original_samples (false),
  text (false),
  gain (1.0),
  loop (-1),
  rate (44100),
  deterministic_random (false)
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
      else if (check_arg (argc, argv, &i, "--export", &opt_arg) || check_arg (argc, argv, &i, "-x", &opt_arg))
        {
          export_wav = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--rate", &opt_arg) || check_arg (argc, argv, &i, "-r", &opt_arg))
        {
          rate = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--samples"))
        {
          enable_original_samples = true;
        }
      else if (check_arg (argc, argv, &i, "--text"))
        {
          text = true;
        }
      else if (check_arg (argc, argv, &i, "--freq", &opt_arg) || check_arg (argc, argv, &i, "-f", &opt_arg))
        {
          freq = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--freq-in", &opt_arg))
        {
          freq_in = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--midi-note", &opt_arg) || check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          freq = freq_from_note (atoi (opt_arg));
        }
      else if (check_arg (argc, argv, &i, "--gain", &opt_arg) || check_arg (argc, argv, &i, "-g", &opt_arg))
        {
          gain = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--loop", &opt_arg))
        {
          loop = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--det-random"))
        {
          deterministic_random = true;
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
  printf ("usage: %s [ <options> ] <smset_file>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                    help for %s\n", options.program_name.c_str());
  printf (" -v, --version                 print version\n");
  printf (" -f, --freq <freq>             specify frequency in Hz\n");
  printf (" -m, --midi-note <note>        specify midi note\n");
  printf (" -x, --export <wav filename>   export to wav file\n");
  printf (" --samples                     use original samples\n");
  printf (" --text                        print output samples as text\n");
  printf (" -g, --gain <gain>             set replay gain\n");
  printf (" --loop <seconds>              enable loop\n");
  printf (" --det-random                  use deterministic/reproducable random generator\n");
  printf (" --rate <sampling rate>        set replay rate manually\n");
  printf ("\n");
}

bool
parse_freq_in (vector<float>& freq_vector)
{
  FILE *freq_file = fopen (options.freq_in.c_str(), "r");
  if (!freq_file)
    {
      fprintf (stderr, "smlive: can't load frequency file %s\n", options.freq_in.c_str());
      return false;
    }

  vector<float> time;
  vector<float> value;

  char buffer[1024];
  while (fgets (buffer, 1024, freq_file) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          char *time_str = strtok (buffer, " \n");
          if (!time_str)
            continue;

          char *value_str = strtok (NULL, " \n");
          if (!value_str)
            continue;
          time.push_back (atof (time_str));
          value.push_back (atof (value_str));
        }
    }
  size_t t = 0;
  for (size_t samples = 0; samples < freq_vector.size(); samples++)
    {
      float xtime = float (samples) / options.rate;
      while (t + 1 < time.size() && time[t + 1] < xtime)
        t++;
      double freq = value[t];
      if (t + 1 < time.size())
        {
          const double frac = (xtime - time[t]) / (time[t + 1] - time[t]);
          freq = (1 - frac) * value[t] + frac * value[t + 1];
        }
      freq_vector[samples] = freq;
      // printf ("%zd %f\n", samples, freq);
    }
  fclose (freq_file);

  return true;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);

  if (argc != 2)
    {
      options.print_usage();
      exit (1);
    }

  WavSet smset;
  if (smset.load (argv[1]) != 0)
    {
      fprintf (stderr, "can't load file %s\n", argv[1]);
      return 1;
    }
  if (options.freq < 0)
    {
      fprintf (stderr, "%s: frequency is required (use -f or -m options)\n", options.program_name.c_str());
      exit (1);
    }

  LiveDecoder decoder (&smset);

  decoder.enable_original_samples (options.enable_original_samples);

  if (options.deterministic_random)
    decoder.set_noise_seed (0x123456);

  size_t len;
  if (options.loop > 0)
    {
      decoder.enable_loop (true);
      len = options.loop * options.rate;
    }
  else
    {
      decoder.enable_loop (false);
      len = 20 * options.rate;                // FIXME: play until end
    }

  vector<float> freq_in;
  if (options.freq_in != "")
    {
      freq_in.resize (len);
      if (!parse_freq_in (freq_in))
        return 1;
    }

  vector<float> audio_out (len);
  decoder.retrigger (0, options.freq, 127, options.rate);
  decoder.process (audio_out.size(), freq_in.size() ? &freq_in[0] : nullptr, 0, &audio_out[0]);

  // hacky way to remove tail silence
  while (!audio_out.empty() && audio_out.back() == 0)
    audio_out.resize (audio_out.size() - 1);

  // apply replay gain
  for (size_t i = 0; i < audio_out.size(); i++)
    audio_out[i] *= options.gain;

  ao_sample_format format = { 0, };

  format.bits = 16;
  format.rate = options.rate;
  format.channels = 1;
  format.byte_format = AO_FMT_NATIVE;

  ao_device *play_device = NULL;

  if (options.export_wav == "")   /* open audio only if we need to play something */
    {
      ao_initialize();

      ao_option *ao_options = NULL;
      int driver_id = ao_default_driver_id ();
      if ((play_device = ao_open_live (driver_id, &format, ao_options)) == NULL)
        {
          fprintf (stderr, "%s: can't open oss output: %s\n", argv[0], strerror (errno));
          exit(1);
        }
    }

  if (options.text)
    {
      for (size_t i = 0; i < audio_out.size(); i++)
        {
          sm_printf ("%.17g\n", audio_out[i]);
        }
    }
  else if (options.export_wav == "")     /* no export -> play */
    {
      bool   clip_err = false;
      size_t pos = 0;
      while (pos < audio_out.size())
        {
          short svalues[1024];

          int todo = std::min (audio_out.size() - pos, size_t (1024));
          for (int i = 0; i < todo; i++)
            {
              float f = audio_out[pos + i];
              float cf = CLAMP (f, -1.0, 1.0);

              if (cf != f && !clip_err)
                {
                  fprintf (stderr, "out of range\n");
                  clip_err = true;
                }

              svalues[i] = cf * 32760;
            }
          //fwrite (svalues, 2, todo, stdout);
          ao_play (play_device, (char *)svalues, 2 * todo);

          pos += todo;
        }
      ao_close (play_device);
    }
  else
    {
      for (size_t i = 0; i < audio_out.size(); i++)
        {
          float f = audio_out[i];
          float cf = CLAMP (f, -1.0, 1.0);
          if (f != cf)
            {
              fprintf (stderr, "smlive: out of range\n");
              break;
            }
        }

      WavData wav_data (audio_out, 1, options.rate);
      if (!wav_data.save (options.export_wav))
        {
          fprintf (stderr, "%s: export to file %s failed: %s\n", options.program_name.c_str(), options.export_wav.c_str(), wav_data.error_blurb());
          exit (1);
        }
    }
}
