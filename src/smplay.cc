/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
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


#include <birnet/birnet.hh>
#include <bse/bse.h>
#include <bse/gslfft.h>
#include <bse/bsemain.h>
#include <bse/bsemathsignal.h>
#include <bse/gsldatahandle.h>
#include <bse/bseblockutils.hh>
#include <sfi/sfiparams.h>
#include "smaudio.hh"
#include "smafile.hh"
#include "frame.hh"
#include <fcntl.h>
#include <errno.h>
#include <ao/ao.h>
#include <assert.h>
#include <math.h>
#include "noisedecoder.hh"
#include "sinedecoder.hh"

#include <list>

#define STWPLAY_VERSION "0.0.1"

using namespace Birnet;
using Stw::Codec::NoiseDecoder;
using Stw::Codec::SineDecoder;
using Stw::Codec::Frame;
using std::max;

struct Options
{
  String	program_name; /* FIXME: what to do with that */
  bool          verbose;
  bool          loop;
  bool          noise_enabled;
  bool          sines_enabled;
  int           rate;
  String        export_wav;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();

  std::list<String>  playlists;
  FILE              *debug;
} options;

#include "stwutils.hh"

Options::Options () :
  program_name ("stwplay"),
  loop (false),
  noise_enabled (true),
  sines_enabled (true),
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
	  printf ("%s %s\n", program_name.c_str(), STWPLAY_VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--verbose"))
	{
	  verbose = true;
	}
      else if (check_arg (argc, argv, &i, "-d"))
	{
          debug = fopen ("/tmp/stwplay.log", "w");
	}
      else if (check_arg (argc, argv, &i, "--list", &opt_arg) || check_arg (argc, argv, &i, "-@", &opt_arg))
	{
	  playlists.push_back (opt_arg);
	}
      else if (check_arg (argc, argv, &i, "--rate", &opt_arg) || check_arg (argc, argv, &i, "-r", &opt_arg))
	{
	  rate = atoi (opt_arg);
	}
      else if (check_arg (argc, argv, &i, "--export", &opt_arg) || check_arg (argc, argv, &i, "-x", &opt_arg))
        {
          export_wav = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--loop") || check_arg (argc, argv, &i, "-l"))
        {
          loop = true;
        }
      else if (check_arg (argc, argv, &i, "--no-noise"))
        {
          noise_enabled = false;
        }
      else if (check_arg (argc, argv, &i, "--no-sines"))
        {
          sines_enabled = false;
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
  g_printerr ("usage: %s [ <options> ] <URI> ...\n", options.program_name.c_str());
  g_printerr ("\n");
  g_printerr ("options:\n");
  g_printerr (" -h, --help                  help for %s\n", options.program_name.c_str());
  g_printerr (" -@, --list <filename>       read files and URIs from \"filename\"\n");
  g_printerr (" --version                   print version\n");
  g_printerr (" --verbose                   print verbose information\n");
  g_printerr (" --rate <sampling rate>      set replay rate manually\n");
  g_printerr (" --no-noise                  disable noise decoder\n");
  g_printerr (" --no-sines                  disable sine decoder\n");
  g_printerr (" --export <wav filename>     export to wav file\n");
  g_printerr ("\n");
}

void debug (const char *dbg, ...) G_GNUC_PRINTF (1, 2);

void
debug (const char *dbg, ...)
{
  va_list ap;

  if (!options.debug)
    return;

  va_start (ap, dbg);
  vfprintf (options.debug, dbg, ap);
  va_end (ap);
}

int
main (int argc, char **argv)
{
  /* init */
  SfiInitValue values[] = {
    { "stand-alone",            "true" }, /* no rcfiles etc. */
    { "wave-chunk-padding",     NULL, 1, },
    { "dcache-block-size",      NULL, 8192, },
    { "dcache-cache-memory",    NULL, 5 * 1024 * 1024, },
    { NULL }
  };
  bse_init_inprocess (&argc, &argv, NULL, values);
  options.parse (&argc, &argv);
  if (argc < 2)
    {
      options.print_usage();
      exit (1);
    }

  /* open input */
  BseErrorType error;

  SpectMorph::Audio audio;
  error = STWAFile::load (argv[1], audio);
  if (error)
    {
      fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], bse_error_blurb (error));
      exit (1);
    }
  fprintf (stderr, "%d blocks\n", audio.contents.size());

  ao_sample_format format;

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

  size_t frame_size = audio.frame_size_ms * format.rate / 1000;
  fprintf (stderr, "frame size: %f ms\n", audio.frame_size_ms);
  fprintf (stderr, "frame size: %zd samples\n", frame_size);

  size_t frame_step = audio.frame_step_ms * format.rate / 1000;
  fprintf (stderr, "frame step: %zd samples\n", frame_step);

  size_t block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  vector<double> window (block_size);
  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }

  fprintf (stderr, "rate: %d Hz\n", format.rate);
  uint loop_point = audio.contents.size() - 5;
  fprintf (stderr, "loop point: %d\n", loop_point);

  vector<float> sample;

  SineDecoder::Mode mode = SineDecoder::MODE_PHASE_SYNC_OVERLAP;
  NoiseDecoder noise_decoder (audio.mix_freq, format.rate);
  SineDecoder  sine_decoder (format.rate, frame_size, frame_step, mode);

  size_t pos = 0;
  size_t end_point = audio.contents.size();

  if (options.loop)
    end_point *= 10;

  // decode one frame before actual data (required for tracking decoder)

  if (mode != SineDecoder::MODE_PHASE_SYNC_OVERLAP)
    {
      Stw::Codec::Frame zero_frame (frame_size);
      Stw::Codec::Frame one_frame (audio.contents[0], frame_size);
      sample.resize (pos + frame_size);

      if (options.sines_enabled)
        {
          sine_decoder.process (zero_frame, one_frame, window);
          for (size_t i = 0; i < frame_size; i++)
            sample[pos + i] += zero_frame.decoded_sines[i];
        }
      pos += frame_step;
    }

  // decode actual data
  for (size_t n = 0; n < end_point; n++)
    {
      size_t n4 = n / 1;
      size_t n4_1 = (n + 1) / 1;

      Stw::Codec::Frame frame (audio.contents[n4 > loop_point ? loop_point : n4], frame_size);
      Stw::Codec::Frame next_frame (audio.contents[n4_1 > loop_point ? loop_point : n4_1], frame_size);

      sample.resize (pos + frame_size);

      if (options.sines_enabled)
        {
          sine_decoder.process (frame, next_frame, window);
          for (size_t i = 0; i < frame_size; i++)
            sample[pos + i] += frame.decoded_sines[i];
        }
      if (options.noise_enabled)
        {
          noise_decoder.process (frame, window);
          for (size_t i = 0; i < frame_size; i++)
            sample[pos + i] += frame.decoded_residue[i];
        }
      pos += frame_step;
    }

  if (options.export_wav == "")     /* no export -> play */
    {
      pos = 0;
      while (pos < sample.size())
        {
          short svalues[1024];

          int todo = std::min (sample.size() - pos, size_t (1024));
          for (int i = 0; i < todo; i++)
            {
              float f = sample[pos + i];
              if (f > 1.0)
                fprintf (stderr, "out of range\n");
              if (f < -1.0)
                fprintf (stderr, "out of range\n");

              svalues[i] = f * 32760;
            }
          //fwrite (svalues, 2, todo, stdout);
          ao_play (play_device, (char *)svalues, 2 * todo);

          pos += todo;
        }
    }
  else /* export wav */
    {
      GslDataHandle *out_dhandle = gsl_data_handle_new_mem (1, 32, options.rate, 44100 / 16 * 2048, sample.size(), &sample[0], NULL);
      BseErrorType error = gsl_data_handle_open (out_dhandle);
      if (error)
        {
          fprintf (stderr, "can not open mem dhandle for exporting wave file\n");
          exit (1);
        }

      int fd = open (options.export_wav.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fd < 0)
        {
          BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
          sfi_error ("export to file %s failed: %s", options.export_wav.c_str(), bse_error_blurb (error));
        }
      int xerrno = gsl_data_handle_dump_wav (out_dhandle, fd, 16, out_dhandle->setup.n_channels, (guint) out_dhandle->setup.mix_freq);
      if (xerrno)
        {
          BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
          sfi_error ("export to file %s failed: %s", options.export_wav.c_str(), bse_error_blurb (error));
        }
    }
}
