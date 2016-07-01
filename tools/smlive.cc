// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavset.hh"
#include "smmain.hh"
#include "smlivedecoder.hh"
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
  bool        enable_original_samples;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("smlive"),
  enable_original_samples (false)
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
      else if (check_arg (argc, argv, &i, "--samples"))
        {
          enable_original_samples = true;
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
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf ("\n");
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

  LiveDecoder decoder (&smset);

  decoder.enable_original_samples (options.enable_original_samples);
  decoder.enable_loop (false);

  const int SR = 48000;
  const float freq = 440;

  vector<float> audio_out (SR * 20);
  decoder.retrigger (0, freq, 127, SR);
  decoder.process (audio_out.size(), 0, 0, &audio_out[0]);

  // hacky way to remove tail silence
  while (!audio_out.empty() && audio_out.back() == 0)
    audio_out.resize (audio_out.size() - 1);

  GslDataHandle *out_dhandle = gsl_data_handle_new_mem (1, 32, SR, SR / 16 * 2048, audio_out.size(), &audio_out[0], NULL);
  Bse::Error error = gsl_data_handle_open (out_dhandle);
  if (error != 0)
    {
      fprintf (stderr, "can not open mem dhandle for exporting wave file\n");
      exit (1);
    }

  ao_sample_format format = { 0, };

  format.bits = 16;
  format.rate = SR; // options.rate;
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

  if (options.export_wav == "")     /* no export -> play */
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
      int fd = open (options.export_wav.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd < 0)
        {
          Bse::Error error = bse_error_from_errno (errno, Bse::Error::FILE_OPEN_FAILED);
          sfi_error ("export to file %s failed: %s", options.export_wav.c_str(), bse_error_blurb (error));
        }
      int xerrno = gsl_data_handle_dump_wav (out_dhandle, fd, 16, out_dhandle->setup.n_channels, (guint) out_dhandle->setup.mix_freq);
      if (xerrno)
        {
          Bse::Error error = bse_error_from_errno (xerrno, Bse::Error::FILE_WRITE_FAILED);
          sfi_error ("export to file %s failed: %s", options.export_wav.c_str(), bse_error_blurb (error));
        }
    }
}
