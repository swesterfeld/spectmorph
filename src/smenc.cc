/* 
 * Copyright (C) 2006-2010 Stefan Westerfeld
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

#include <sfi/sfistore.h>
#include <bse/bsemain.h>
#include <bse/bseloader.h>
#include <bse/gslfft.h>
#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>
#include <list>
#include <unistd.h>
#include <bse/gsldatautils.h>
#include <assert.h>
#include <complex>

#include "smaudio.hh"
#include "smafile.hh"
#include "smencoder.hh"
#include "config.h"

using std::string;
using std::vector;
using std::list;
using std::min;
using std::max;

using namespace Birnet;

using SpectMorph::AudioBlock;
using SpectMorph::EncoderParams;
using SpectMorph::Encoder;
using SpectMorph::Tracksel;

float
freqFromNote (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

struct Options
{
  string	program_name; /* FIXME: what to do with that */
  bool          strip_models;
  float         fundamental_freq;
  int           optimization_level;
  FILE         *debug;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;

#include "stwutils.hh"

Options::Options ()
{
  program_name = "smenc";
  fundamental_freq = 0; // unset
  debug = 0;
  optimization_level = 0;
  strip_models = false;
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

void
wintrans (const vector<float>& window)
{
  vector<double> in (window.begin(), window.end());
  vector<double> out;
  const int zpad = 4;

  in.resize (in.size() * 4);
  out.resize (in.size());

  double amp = 0;
  for (size_t i = 0; i < in.size(); i++)
    amp += in[i];

  for (size_t i = 0; i < in.size(); i++)
    in[i] /= amp;

  gsl_power2_fftar (in.size(), &in[0], &out[0]);
  for (int i = 0; i < out.size(); i += 2)
    {
      double re = out[i];
      double im = out[i + 1];
      double mag = sqrt (re * re + im * im);
      printf ("%d %g\n", i / 2, mag);
    }
}

size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

int
main (int argc, char **argv)
{
  EncoderParams enc_params;

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

  if (argc != 3)
    {
      options.print_usage();
      exit (1);
    }

  /* open input */
  BseErrorType error;

  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (argv[1], &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  const uint64 n_values = gsl_data_handle_length (dhandle);
  const double mix_freq = gsl_data_handle_mix_freq (dhandle);
  const int    zeropad  = 4;

  enc_params.mix_freq = mix_freq;
  enc_params.zeropad  = zeropad;
  enc_params.frame_size_ms = 40;
  if (options.fundamental_freq > 0)
    {
      enc_params.frame_size_ms = max (enc_params.frame_size_ms, 1000 / options.fundamental_freq * 4);
    }
  enc_params.frame_step_ms = enc_params.frame_size_ms / 4.0;

  const size_t  frame_size = make_odd (mix_freq * 0.001 * enc_params.frame_size_ms);
  const size_t  frame_step = mix_freq * 0.001 * enc_params.frame_step_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  enc_params.frame_step = frame_step;
  enc_params.frame_size = frame_size;
  enc_params.block_size = block_size;

  if (options.fundamental_freq > 0)
    fprintf (stderr, "fundamental freq = %f\n", options.fundamental_freq);

  Encoder encoder (enc_params);

  vector<float>  block (block_size);
  vector<float> window (block.size());
  vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);
  vector<AudioBlock>& audio_blocks = encoder.audio_blocks;

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }

  //wintrans (window);

  encoder.compute_stft (dhandle, window);
  // Track frequencies step #0: find maximum of all values
  // Track frequencies step #1: search for local maxima as potential track candidates
  encoder.search_local_maxima();

  // Track frequencies step #2: link lists together
  encoder.link_partials();
  // Track frequencies step #3: discard edges where -25 dB is not exceeded once
  encoder.validate_partials();

  encoder.optimize_partials (window, options.optimization_level);

  encoder.spectral_subtract (window);
  encoder.approx_noise();

  if (options.strip_models)
    {
      for (size_t i = 0; i < audio_blocks.size(); i++)
        {
          audio_blocks[i].debug_samples.clear();
          audio_blocks[i].original_fft.clear();
        }
    }
  encoder.save (argv[2], options.fundamental_freq);
}
