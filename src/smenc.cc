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

#define STWENC_VERSION "0.0.1"

using std::string;
using std::vector;
using std::list;
using std::min;
using std::max;

using namespace Birnet;

using SpectMorph::AudioBlock;

namespace ublas = boost::numeric::ublas;
using ublas::matrix;

/* Matrix inversion routine.
  Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
template<class T>
bool invert_matrix (const ublas::matrix<T>& input, ublas::matrix<T>& inverse)
{
  using namespace boost::numeric::ublas;
  typedef permutation_matrix<std::size_t> pmatrix;
  // create a working copy of the input
  matrix<T> A(input);
  // create a permutation matrix for the LU-factorization
  pmatrix pm(A.size1());

  // perform LU-factorization
  int res = lu_factorize(A,pm);
  if( res != 0 ) return false;

  // create identity matrix of "inverse"
  inverse.assign(ublas::identity_matrix<T>(A.size1()));

  // backsubstitute to get the inverse
  lu_substitute(A, pm, inverse);

  return true;
}

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
	  printf ("%s %s\n", program_name.c_str(), STWENC_VERSION);
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

struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
  double   phasea;    /* sine amplitude */
  double   phaseb;    /* cosine amplitude */
  bool     is_harmonic;
  Tracksel *prev, *next;
};

bool
check_harmonic (double freq, double& new_freq, double mix_freq)
{
  if (options.fundamental_freq > 0)
    {
      for (int i = 1; i < 100; i++)
	{
	  double base_freq = (mix_freq/2048*16);
	  double harmonic_freq = i * base_freq;
	  double diff = fabs (harmonic_freq - freq) / base_freq;
	  if (diff < 0.125)
	    {
	      new_freq = harmonic_freq;
	      return true;
	    }
	}
    }
  new_freq = freq;
  return false;
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

void
approximate_noise_spectrum (int frame,
                            const vector<double>& spectrum,
			    vector<double>& envelope)
{
  g_return_if_fail ((spectrum.size() - 2) % envelope.size() == 0);
  int section_size = (spectrum.size() - 2) / envelope.size() / 2;
  int d = 0;
  for (size_t t = 0; t < spectrum.size(); t += 2)
    {
      debug ("noise2red:%d %f\n", frame, sqrt (spectrum[t] * spectrum[t] + spectrum[t + 1] * spectrum[t + 1]));
    }
  for (vector<double>::iterator ei = envelope.begin(); ei != envelope.end(); ei++)
    {
      double max_mag = 0;

      /* represent each spectrum section by its maximum value */
      for (int i = 0; i < section_size; i++)
	{
	  max_mag = max (max_mag, sqrt (spectrum[d] * spectrum[d] + spectrum[d + 1] * spectrum[d + 1]));
	  d += 2;
	}
      *ei = bse_db_from_factor (max_mag, -200);
      debug ("noisered:%d %f\n", frame, *ei);
    }
}

void
xnoise_envelope_to_spectrum (const vector<double>& envelope,
			    vector<double>& spectrum)
{
  g_return_if_fail (spectrum.size() == 2050);
  int section_size = 2048 / envelope.size();
  for (int d = 0; d < spectrum.size(); d += 2)
    {
      if (d <= section_size / 2)
	{
	  spectrum[d] = bse_db_to_factor (envelope[0]);
	}
      else if (d >= spectrum.size() - section_size / 2 - 2)
	{
	  spectrum[d] = bse_db_to_factor (envelope[envelope.size() - 1]);
	}
      else
	{
	  int dd = d - section_size / 2;
	  double f = double (dd % section_size) / section_size;
	  spectrum[d] = bse_db_to_factor (envelope[dd / section_size] * (1 - f)
                                        + envelope[dd / section_size + 1] * f);
	}
      spectrum[d+1] = 0;
      debug ("noiseint %f\n", spectrum[d]);
    }
}

void
refine_sine_params (AudioBlock& audio_block, double mix_freq)
{
  const size_t freq_count = audio_block.freqs.size();
  const size_t signal_size = audio_block.debug_samples.size();

  if (freq_count == 0)
    return;

  // input: M x N matrix containing base of a vector subspace, consisting of N M-dimesional vectors
  matrix<double, ublas::column_major> A (signal_size, freq_count * 2); 

  boost::numeric::ublas::vector<double> S (freq_count * 2);                    // output: <not used> N element vector
  matrix<double, ublas::column_major> U (signal_size, freq_count * 2);         // output: M x N matrix containing orthogonal base of the vector subspace, consisting of N M-dimensional vectors
  matrix<double, ublas::column_major> Vt (freq_count * 2,freq_count * 2);      // output: <not used> N x N matrix

  for (int i = 0; i < freq_count; i++)
    {
      for (size_t x = 0; x < signal_size; x++)
        {
          A(x, i * 2) = sin (x / mix_freq * audio_block.freqs[i] * 2 * M_PI);
          A(x, i * 2 + 1) = cos (x / mix_freq * audio_block.freqs[i] * 2 * M_PI);
        }
    }

  // compute SVD to obtain an orthogonal base

  // A is modified during SVD, so we copy A before doing it
  matrix<double, ublas::column_major> Acopy = A;
  boost::numeric::bindings::lapack::gesdd (Acopy, S, U, Vt); 

  // compute coordinates of the signal in vector space of orthogonal base
  matrix<double> ortho (freq_count * 2, 1);
  for (int a = 0; a < freq_count * 2; a++)
    {
      double d = 0;
      for (size_t x = 0; x < signal_size; x++)
        d += U(x,a) * audio_block.debug_samples[x]; // compute dot product
      ortho (a,0) = d;
    }

  // fill M with the transformation from sine/cosine base to orthogonal base
  matrix<double> M (freq_count * 2, freq_count * 2);
  for (int a = 0; a < freq_count * 2; a++)
    {
      for (int i = 0; i < freq_count * 2; i++)
        {
          double d = 0;
          for (size_t x = 0; x < signal_size; x++)
            d += U(x,i) * A(x,a);                   // compute dot product
          M(i,a) = d;
        }
    }

  // invert matrix to get the transformation from orthogonal base to sine/cosine base
  matrix<double> MI (freq_count * 2, freq_count * 2);
  invert_matrix (M, MI);

  // transform orthogonal coordinates into sine/cosine coordinates
  matrix<double> MIo = prod (MI, ortho);

  // store improved phase/amplitude information
  assert (audio_block.phases.size() == freq_count * 2);
  for (int i = 0; i < freq_count * 2; i++)
    audio_block.phases[i] = MIo(i, 0);
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
                if (frame == 20)
                  printf ("%d %f %f\n", partial, sines[n], audio_block.debug_samples[n]);
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
}

struct EncoderParams
{
  float mix_freq;       /* mix_freq of the original audio file */
  float frame_step_ms;  /* step size for analysis frames */
  float frame_size_ms;  /* size of one analysis frame */
  int   zeropad;        /* lower bound for zero padding during analysis */
};

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

  const size_t  frame_step = mix_freq * 0.001 * enc_params.frame_step_ms;

  if (options.fundamental_freq > 0)
    fprintf (stderr, "fundamental freq = %f\n", options.fundamental_freq);
  fprintf (stderr, "frame_size = %zd (%f ms)\n", frame_size, enc_params.frame_size_ms);
  vector<float>  block (block_size);
  vector<float> window (block.size());
  vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);
  vector<double> last_phase (block_size * zeropad + 2);
  vector<AudioBlock> audio_blocks;

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
  double max_mag = 0;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
	  max_mag = std::max (max_mag, magnitude (audio_blocks[n].meaning.begin() + d));
	}
    }

  // Track frequencies step #1: search for local maxima as potential track candidates
  vector< vector<Tracksel> > frame_tracksels (audio_blocks.size()); /* Analog to Canny Algorithms edgels */
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      vector<double> mag_values (audio_blocks[n].meaning.size() / 2);
      for (size_t d = 0; d < block_size * zeropad; d += 2)
        mag_values[d / 2] = magnitude (audio_blocks[n].meaning.begin() + d);

      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
#if 0
	  double phase = atan2 (*(audio_blocks[n]->meaning.begin() + d),
	                        *(audio_blocks[n]->meaning.begin() + d + 1)) / 2 / M_PI;  /* range [-0.5 .. 0.5] */
#endif
          if (mag_values[d/2] > mag_values[d/2-1] && mag_values[d/2] > mag_values[d/2+1])   /* search for peaks in fft magnitudes */
            {
              /* need [] operater in fblock */
              double mag2 = bse_db_from_factor (mag_values[d / 2] / max_mag, -100);
              debug ("dbspectrum:%zd %f\n", n, mag2);
              if (mag2 > -90)
                {
                  size_t ds, de;
                  for (ds = d / 2 - 1; ds > 0 && mag_values[ds] < mag_values[ds + 1]; ds--);
                  for (de = d / 2 + 1; de < (mag_values.size() - 1) && mag_values[de] < mag_values[de + 1]; de++);
                  if (de - ds > 10)
                    {
                      printf ("%d %f\n", de-ds, mag2);
                      double mag1 = bse_db_from_factor (mag_values[d / 2 - 1] / max_mag, -100);
                      double mag3 = bse_db_from_factor (mag_values[d / 2 + 1] / max_mag, -100);
                      //double freq = d / 2 * mix_freq / (block_size * zeropad); /* bin frequency */

                      /* a*x^2 + b*x + c */
                      double a = (mag1 + mag3 - 2*mag2) / 2;
                      double b = mag3 - mag2 - a;
                      double c = mag2;
                      //printf ("f%d(x) = %f * x * x + %f * x + %f\n", n, a, b, c);
                      double x_max = -b / (2 * a);
                      //printf ("x_max%d=%f\n", n, x_max);
                      double tfreq = (d / 2 + x_max) * mix_freq / (block_size * zeropad);

                      double peak_mag_db = a * x_max * x_max + b * x_max + c;
                      double peak_mag = bse_db_to_factor (peak_mag_db) * max_mag;

                      // use the interpolation formula for the complex values to find the phase
                      std::complex<double> c1 (audio_blocks[n].meaning[d-2], audio_blocks[n].meaning[d-1]);
                      std::complex<double> c2 (audio_blocks[n].meaning[d], audio_blocks[n].meaning[d+1]);
                      std::complex<double> c3 (audio_blocks[n].meaning[d+2], audio_blocks[n].meaning[d+3]);
                      std::complex<double> ca = (c1 + c3 - 2.0*c2) / 2.0;
                      std::complex<double> cb = c3 - c2 - ca;
                      std::complex<double> cc = c2;
                      std::complex<double> interp_c = ca * x_max * x_max + cb * x_max + cc;
    /*
                      if (mag2 > -20)
                        printf ("%f %f %f %f %f\n", phase, last_phase[d], phase_diff, phase_diff * mix_freq / (block_size * zeropad) * overlap, tfreq);
    */
                      Tracksel tracksel;
                      tracksel.frame = n;
                      tracksel.d = d;
                      tracksel.freq = tfreq;
                      tracksel.mag = peak_mag / frame_size * zeropad;
                      tracksel.mag2 = mag2;
                      tracksel.phasea = interp_c.real() / frame_size * zeropad;
                      tracksel.phaseb = interp_c.imag() / frame_size * zeropad;
                      tracksel.next = 0;
                      tracksel.prev = 0;

                      double dummy_freq;
                      tracksel.is_harmonic = check_harmonic (tracksel.freq, dummy_freq, mix_freq);
                      // FIXME: need a different criterion here
                      // mag2 > -30 doesn't track all partials
                      // mag2 > -60 tracks lots of junk, too
                      if ((mag2 > -90 || tracksel.is_harmonic) && tracksel.freq > 10)
                        frame_tracksels[n].push_back (tracksel);
                    }
                }
#if 0
              last_phase[d] = phase;
#endif
            }
	}
    }

  // Track frequencies step #2: link lists together
  for (size_t n = 0; n + 1 < audio_blocks.size(); n++)
    {
      Tracksel *crosslink_i, *crosslink_j;
      do
	{
	  /* find a good pair of edges to link together */
	  crosslink_i = crosslink_j = 0;
	  double best_crossmag = -200;
	  vector<Tracksel>::iterator i, j;
	  for (i = frame_tracksels[n].begin(); i != frame_tracksels[n].end(); i++)
	    {
	      for (j = frame_tracksels[n + 1].begin(); j != frame_tracksels[n + 1].end(); j++)
		{
		  if (!i->next && !j->prev)
		    {
		      if (fabs (i->freq - j->freq) / i->freq < 0.05) /* 5% frequency derivation */
			{
			  double crossmag = i->mag2 + j->mag2;
			  if (crossmag > best_crossmag)
			    {
			      best_crossmag = crossmag;
			      crosslink_i = &(*i);
			      crosslink_j = &(*j);
			    }
			}
		    }
		}
	    }
	  if (crosslink_i && crosslink_j)
	    {
	      crosslink_i->next = crosslink_j;
	      crosslink_j->prev = crosslink_i;
	    }
	} while (crosslink_i);
    }
  // Track frequencies step #3: discard edges where -25 dB is not exceeded once
  map<Tracksel *, bool> processed_tracksel;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      vector<Tracksel>::iterator i, j;
      for (i = frame_tracksels[n].begin(); i != frame_tracksels[n].end(); i++)
	{
	  if (!processed_tracksel[&(*i)])
	    {
	      double biggest_mag = -100;
	      bool   is_harmonic = false;
	      for (Tracksel *t = &(*i); t->next; t = t->next)
		{
		  biggest_mag = std::max (biggest_mag, t->mag2);
		  if (t->is_harmonic)
		    is_harmonic = true;
		  processed_tracksel[t] = true;
		}
	      if (biggest_mag > -90 || is_harmonic)
		{
		  for (Tracksel *t = &(*i); t->next; t = t->next)
		    {
#if 0
		      double new_freq;
		      if (check_harmonic (t->freq, new_freq, mix_freq))
			t->freq = new_freq;
#endif

		      audio_blocks[t->frame].freqs.push_back (t->freq);
		      //audio_blocks[t->frame].phases.push_back (t->mag);
		      audio_blocks[t->frame].phases.push_back (t->phaseb);
		      audio_blocks[t->frame].phases.push_back (t->phasea);
                      // empiric phasea / phaseb
#if 0
                      double esa = 0;
                      double eca = 0;
                      double phase = 0;
                      for (size_t x = 0; x < frame_size; x++)
                        {
                          double v = audio_blocks[t->frame].debug_samples[x];
                          esa += sin (phase) * v / 1000;
                          eca += cos (phase) * v / 1000;
                          phase += t->freq / mix_freq * 2.0 * M_PI;
                        }
#endif
		      //audio_blocks[t->frame].phases.push_back (esa);
		      //audio_blocks[t->frame].phases.push_back (eca);
                      //printf ("%f %f %f %f\n", atan (t->phaseb / t->phasea), atan (esa / eca), t->phaseb, esa);
#if 0  /* better: spectrum subtraction */
		      for (int clean = t->d - 4; clean <= t->d + 4; clean += 2)
			{
			  if (clean >= 0 && clean < block_size)
			    {
			      *(audio_blocks[t->frame]->meaning.begin() + clean) = 0.0;
			      *(audio_blocks[t->frame]->meaning.begin() + clean + 1) = 0.0;
			    }
			}
#endif
		    }
		}
	    }
	}
    }

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      refine_sine_params_fast (audio_blocks[frame], mix_freq, frame);
      if (options.optimize)
        {
          refine_sine_params (audio_blocks[frame], mix_freq);
          printf ("refine: %2.3f %%\r", frame * 100.0 / audio_blocks.size());
          fflush (stdout);
        }
      remove_small_partials (audio_blocks[frame]);
#if 0
      vector<float> sines (frame_size);
      vector<float> good_freqs;
      vector<float> good_phases;

      double max_mag;
      size_t partial = 0;
      do
        {
          max_mag = 0;
          // search biggest partial
          for (size_t i = 0; i < audio_blocks[frame].freqs.size(); i++)
            {
              double p_re = audio_blocks[frame].phases[2 * i];
              double p_im = audio_blocks[frame].phases[2 * i + 1];
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
              double smag = audio_blocks[frame].phases[2 * partial];
              double cmag = audio_blocks[frame].phases[2 * partial + 1];
              double f = audio_blocks[frame].freqs[partial];

              audio_blocks[frame].phases[2 * partial] = 0;
              audio_blocks[frame].phases[2 * partial + 1] = 0;

              double phase = 0;
              // determine "perfect" phase and magnitude instead of using interpolated fft phase
              smag = 0;
              cmag = 0;
              double snorm = 0, cnorm = 0;
              for (size_t n = 0; n < frame_size; n++)
                {
                  double v = audio_blocks[frame].debug_samples[n];
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
              double delta = float_vector_delta (sines, audio_blocks[frame].debug_samples);
              phase = 0;
              for (size_t n = 0; n < frame_size; n++)
                {
                  phase += f / mix_freq * 2.0 * M_PI;
                  sines[n] += sin (phase) * smag;
                  sines[n] += cos (phase) * cmag;
                }
              double new_delta = float_vector_delta (sines, audio_blocks[frame].debug_samples);
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

      audio_blocks[frame].freqs = good_freqs;
      audio_blocks[frame].phases = good_phases;
#endif

      fill (in.begin(), in.end(), 0);

      // compute spectrum of isolated sine frequencies from audio spectrum
      for (size_t i = 0; i < audio_blocks[frame].freqs.size(); i++)
	{
	  double phase = 0;
	  for (size_t k = 0; k < block_size; k++)
	    {
	      double freq = audio_blocks[frame].freqs[i];
	      double re = audio_blocks[frame].phases[i * 2];
	      double im = audio_blocks[frame].phases[i * 2 + 1];
	      double mag = sqrt (re * re + im * im);
	      phase += freq / mix_freq * 2 * M_PI;
	      in[k] += mag * sin (phase) * window[k];
	    }
	}
      gsl_power2_fftar (block_size * zeropad, &in[0], &out[0]);
      out[block_size * zeropad] = out[1];
      out[block_size * zeropad + 1] = 0;
      out[1] = 0;

      // subtract spectrum from audio spectrum
      for (size_t d = 0; d < block_size * zeropad; d += 2)
	{
	  double re = out[d], im = out[d + 1];
	  double sub_mag = sqrt (re * re + im * im);
	  debug ("subspectrum:%lld %g\n", frame, sub_mag);

	  double mag = magnitude (audio_blocks[frame].meaning.begin() + d);
	  debug ("spectrum:%lld %g\n", frame, mag);
	  if (mag > 0)
	    {
	      audio_blocks[frame].meaning[d] /= mag;
	      audio_blocks[frame].meaning[d + 1] /= mag;
	      mag -= sub_mag;
	      if (mag < 0)
		mag = 0;
	      audio_blocks[frame].meaning[d] *= mag;
	      audio_blocks[frame].meaning[d + 1] *= mag;
	    }
	  debug ("finalspectrum:%lld %g\n", frame, mag);
	}
    }

#if 0 // FIXME
  if (options.debug)
    {
      /* residual resynthesis */
      FILE *res = fopen ("/tmp/stwenc.res", "w");
      vector<double> residual;
      for (uint64 frame = 0; frame < audio_blocks.length(); frame++)
	{
	  vector<double> res (audio_blocks[frame]->meaning.begin(), audio_blocks[frame]->meaning.end());
	  vector<double> rout (res.size());
	  gsl_power2_fftsr (block_size * zeropad, &res[0], &rout[0]);
	  for (int i = 0; i < block_size; i++)
	    {
	      size_t pos = frame * block_size / overlap + i;
	      residual.resize (max (residual.size(), pos + 1));
	      residual[pos] += rout[i] * window[i];
	    }
	}
      for (uint64 i = 0; i < residual.size(); i++)
	{
	  short s = residual[i] * 32760;
	  fwrite (&s, 2, 1, res);
	}
      fclose (res);
    }
#endif

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      vector<double> noise_envelope (256);
      vector<double> spectrum (audio_blocks[frame].meaning.begin(), audio_blocks[frame].meaning.end());

      approximate_noise_spectrum (frame, spectrum, noise_envelope);

      vector<double> approx_spectrum (2050);
      xnoise_envelope_to_spectrum (noise_envelope, approx_spectrum);
      for (int i = 0; i < 2048; i += 2)
	debug ("spect_approx:%lld %g\n", frame, approx_spectrum[i]);
      audio_blocks[frame].meaning.resize (noise_envelope.size());
      copy (noise_envelope.begin(), noise_envelope.end(), audio_blocks[frame].meaning.begin());
    }

  SpectMorph::Audio audio;
  audio.fundamental_freq = options.fundamental_freq;
  audio.mix_freq = enc_params.mix_freq;
  audio.frame_size_ms = enc_params.frame_size_ms;
  audio.frame_step_ms = enc_params.frame_step_ms;
  audio.zeropad = zeropad;
  audio.contents = audio_blocks;
  STWAFile::save (argv[2], audio);
}
