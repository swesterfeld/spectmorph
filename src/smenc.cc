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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/bindings/lapack/lapack.hpp>

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

namespace ublas = boost::numeric::ublas;
using ublas::matrix;

float
freqFromNote (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

struct Options
{
  string	program_name; /* FIXME: what to do with that */
  bool          verbose;
  uint          quantize_entries;
  float         fundamental_freq;
  bool          optimize;
  FILE         *debug;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;

#include "stwutils.hh"

Options::Options ()
{
  program_name = "stwenc";
  fundamental_freq = 0; // unset
  debug = 0;
  quantize_entries = 0;
  verbose = false;
  optimize = false;
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
      else if (check_arg (argc, argv, &i, "--verbose"))
	{
	  verbose = true;
	}
      else if (check_arg (argc, argv, &i, "-f", &opt_arg))
	{
	  fundamental_freq = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          fundamental_freq = freqFromNote (atoi (opt_arg));
        }
      else if (check_arg (argc, argv, &i, "-O"))
        {
          optimize = true;
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
  g_printerr ("usage: %s [ <options> ] <src_audio_file> <dest_stwa_file>\n", options.program_name.c_str());
  g_printerr ("\n");
  g_printerr ("options:\n");
  g_printerr (" -h, --help                  help for %s\n", options.program_name.c_str());
  g_printerr (" -@, --list <filename>       read files and URIs from \"filename\"\n");
  g_printerr (" --version                   print version\n");
  g_printerr (" --verbose                   print verbose information\n");
  g_printerr (" -f <freq>                   specify fundamental frequency in Hz\n");
  g_printerr (" -m <note>                   specify midi note for fundamental frequency\n");
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

double
magnitude (vector<float>::iterator i)
{
  return sqrt (*i * *i + *(i+1) * *(i+1));
}

double
float_vector_delta (const vector<float>& a, const vector<float>& b)
{
  assert (a.size() == b.size());

  double d = 0;
  for (size_t i = 0; i < a.size(); i++)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  return d;
}

// find best fit of amplitudes / phases to the observed signal
void
refine_sine_params (AudioBlock& audio_block, double mix_freq, const vector<float>& window)
{
  const size_t freq_count = audio_block.freqs.size();
  const size_t signal_size = audio_block.debug_samples.size();

  if (freq_count == 0)
    return;

  // input: M x N matrix containing base of a vector subspace, consisting of N M-dimesional vectors
  matrix<double, ublas::column_major> A (signal_size, freq_count * 2); 
  for (int i = 0; i < freq_count; i++)
    {
      double phase = 0;
      const double delta_phase = audio_block.freqs[i] * 2 * M_PI / mix_freq;

      for (size_t x = 0; x < signal_size; x++)
        {
          double s, c;
          sincos (phase, &s, &c);
          A(x, i * 2) = s * window[x];
          A(x, i * 2 + 1) = c * window[x];
          phase += delta_phase;
        }
    }

  // input: M dimensional target vector
  ublas::vector<double> b (signal_size);
  for (size_t x = 0; x < signal_size; x++)
    b[x] = audio_block.debug_samples[x] * window[x];

  // generalized least squares algorithm minimizing residual r = Ax - b
  boost::numeric::bindings::lapack::gels ('N', A, b, boost::numeric::bindings::lapack::optimal_workspace());

  // => output: vector containing optimal choice for phases and magnitudes
  for (int i = 0; i < freq_count * 2; i++)
    audio_block.phases[i] = b[i];
}

void
refine_sine_params_fast (AudioBlock& audio_block, double mix_freq, int frame)
{
  const size_t frame_size = audio_block.debug_samples.size();

  vector<float> sines (frame_size);
  vector<float> good_freqs;
  vector<float> good_phases;

  double max_mag;
  size_t partial = 0;
  do
    {
      max_mag = 0;
      // search biggest partial
      for (size_t i = 0; i < audio_block.freqs.size(); i++)
        {
          double p_re = audio_block.phases[2 * i];
          double p_im = audio_block.phases[2 * i + 1];
          double p_mag = sqrt (p_re * p_re + p_im * p_im);
          if (p_mag > max_mag)
            {
              partial = i;
              max_mag = p_mag;
            }
        }
        // compute reconstruction of that partial
        if (max_mag > 0)
          {
            // remove partial, so we only do each partial once
            double smag = audio_block.phases[2 * partial];
            double cmag = audio_block.phases[2 * partial + 1];
            double f = audio_block.freqs[partial];

            audio_block.phases[2 * partial] = 0;
            audio_block.phases[2 * partial + 1] = 0;

            double phase;
            // determine "perfect" phase and magnitude instead of using interpolated fft phase
            smag = 0;
            cmag = 0;
            double snorm = 0, cnorm = 0;
            for (size_t n = 0; n < frame_size; n++)
              {
                double v = audio_block.debug_samples[n] - sines[n];
                phase = ((n - (frame_size - 1) / 2.0) * f) / mix_freq * 2.0 * M_PI;
                smag += sin (phase) * v;
                cmag += cos (phase) * v;
                snorm += sin (phase) * sin (phase);
                cnorm += cos (phase) * cos (phase);
              }
            smag /= snorm;
            cmag /= cnorm;

            double magnitude = sqrt (smag * smag + cmag * cmag);
            phase = atan2 (smag, cmag);
            phase += (frame_size - 1) / 2.0 / mix_freq * f * 2 * M_PI;
            smag = sin (phase) * magnitude;
            cmag = cos (phase) * magnitude;

            vector<float> old_sines = sines;
            double delta = float_vector_delta (sines, audio_block.debug_samples);
            phase = 0;
            for (size_t n = 0; n < frame_size; n++)
              {
                sines[n] += sin (phase) * smag;
                sines[n] += cos (phase) * cmag;
                phase += f / mix_freq * 2.0 * M_PI;
              }
            double new_delta = float_vector_delta (sines, audio_block.debug_samples);
            if (new_delta > delta)      // approximation is _not_ better
              sines = old_sines;
            else
              {
                good_freqs.push_back (f);
                good_phases.push_back (smag);
                good_phases.push_back (cmag);
              }
          }
      }
  while (max_mag > 0);

  audio_block.freqs = good_freqs;
  audio_block.phases = good_phases;
}

static void
remove_small_partials (AudioBlock& audio_block)
{
  /*
   * this function mainly serves to eliminate side peaks introduced by windowing
   * since these side peaks are typically much smaller than the main peak, we can
   * get rid of them by comparing peaks to the nearest peak, and removing them
   * if the nearest peak is much larger
   */
  vector<double> dbmags;
  for (vector<float>::iterator pi = audio_block.phases.begin(); pi != audio_block.phases.end(); pi += 2)
    dbmags.push_back (bse_db_from_factor (magnitude (pi), -200));

  vector<bool> remove (dbmags.size());

  for (size_t i = 0; i < dbmags.size(); i++)
    {
      for (size_t j = 0; j < dbmags.size(); j++)
        {
          if (i != j)
            {
              double octaves = log (abs (audio_block.freqs[i] - audio_block.freqs[j])) / log (2);
              double mask = -30 - 15 * octaves; /* theoretical values -31 and -18 */
              if (dbmags[j] < dbmags[i] + mask)
                ;
                // remove[j] = true;
            }
        }
    }

  vector<float> good_freqs;
  vector<float> good_phases;

  for (size_t i = 0; i < dbmags.size(); i++)
    {
      if (!remove[i])
        {
          good_freqs.push_back (audio_block.freqs[i]);
          good_phases.push_back (audio_block.phases[i * 2]);
          good_phases.push_back (audio_block.phases[i * 2 + 1]);
        }
    }
  audio_block.freqs = good_freqs;
  audio_block.phases = good_phases;
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

  const size_t  frame_size = mix_freq * 0.001 * enc_params.frame_size_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  enc_params.frame_size = frame_size;
  enc_params.block_size = block_size;

  const size_t  frame_step = mix_freq * 0.001 * enc_params.frame_step_ms;

  if (options.fundamental_freq > 0)
    fprintf (stderr, "fundamental freq = %f\n", options.fundamental_freq);
  fprintf (stderr, "frame_size = %zd (%f ms)\n", frame_size, enc_params.frame_size_ms);

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

  fprintf (stderr, "%s: %d channels, %lld values\n", argv[1], gsl_data_handle_n_channels (dhandle), n_values);

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }
  fprintf (stderr, "block_size = %lld\n", block_size);

  //wintrans (window);

  for (uint64 pos = 0; pos < n_values; pos += frame_step)
    {
      AudioBlock audio_block;

      /* read data from file, zeropad last blocks */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      if (r != block.size())
        {
          while (r < block.size())
            block[r++] = 0;
        }
      vector<float> debug_samples (block.begin(), block.end());
      Bse::Block::mul (block_size, &block[0], &window[0]);
      std::copy (block.begin(), block.end(), in.begin());    /* in is zeropadded */

      gsl_power2_fftar (block_size * zeropad, &in[0], &out[0]);
      out[block_size * zeropad] = out[1];
      out[block_size * zeropad + 1] = 0;
      out[1] = 0;

      audio_block.meaning.assign (out.begin(), out.end()); // <- will be overwritten by noise spectrum later on
      audio_block.original_fft.assign (out.begin(), out.end());
      audio_block.debug_samples.assign (debug_samples.begin(), debug_samples.begin() + frame_size);

      audio_blocks.push_back (audio_block);
    }
  // Track frequencies step #0: find maximum of all values
  // Track frequencies step #1: search for local maxima as potential track candidates
  vector< vector<Tracksel> > frame_tracksels (audio_blocks.size()); /* Analog to Canny Algorithms edgels */
  encoder.search_local_maxima (frame_tracksels);

  // Track frequencies step #2: link lists together
  encoder.link_partials (frame_tracksels);
  // Track frequencies step #3: discard edges where -25 dB is not exceeded once
  encoder.validate_partials (frame_tracksels);
  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      refine_sine_params_fast (audio_blocks[frame], mix_freq, frame);
      if (options.optimize)
        {
          refine_sine_params (audio_blocks[frame], mix_freq, window);
          printf ("refine: %2.3f %%\r", frame * 100.0 / audio_blocks.size());
          fflush (stdout);
        }
      remove_small_partials (audio_blocks[frame]);

      fill (in.begin(), in.end(), 0);
    }
  encoder.spectral_subtract (window);
  encoder.approx_noise();
  encoder.save (argv[2], options.fundamental_freq);
}
