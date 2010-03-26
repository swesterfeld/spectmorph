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

#include "stwaudio.hh"
#include "stwafile.hh"

#define STWENC_VERSION "0.0.1"

using std::string;
using std::vector;
using std::list;
using std::min;
using std::max;

using namespace Birnet;

using Stw::Codec::AudioBlockSeq;
using Stw::Codec::AudioBlock;

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
  FILE         *debug;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();

  list<string>  encodelists;
} options;

#include "stwutils.hh"

Options::Options ()
{
  program_name = "stwenc";
  fundamental_freq = 0; // unset
  debug = 0;
  quantize_entries = 0;
  verbose = false;
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
      else if (check_arg (argc, argv, &i, "-q", &opt_arg))
	{
	  quantize_entries = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-f", &opt_arg))
	{
	  fundamental_freq = atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          fundamental_freq = freqFromNote (atoi (opt_arg));
        }
      else if (check_arg (argc, argv, &i, "--list", &opt_arg) || check_arg (argc, argv, &i, "-@", &opt_arg))
	{
	  options.encodelists.push_back (opt_arg);
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

const uint64   n_dimensions = 7;

Stw::Codec::AudioBlockSeq audio_blocks;
vector< vector<double> > codebook;
vector< vector<double> > codebook_centroid;
vector< int > codebook_usage (1);
size_t biggest_usage = 0;
size_t biggest_usage_k = -1;

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

/* search nearest codebook entry for audio_block n */
size_t
search_nearest_codebook_entry (size_t n)
{
  assert (false);   // FIXME
#if 0
  double min_dist = 1e30;
  size_t dist_k = -1;
  for (size_t k = 0; k < codebook.size(); k++)
    {
      double dist = 0;
      for (size_t i = 0; i < block_size + 2; i++)
        {
          double d = *(audio_blocks[n]->meaning.begin() + i) - codebook[k][i];
          dist += d * d;
        }
      if (dist < min_dist)
        {
          min_dist = dist;
          dist_k = k;
        }
    }
  return dist_k;
#endif
}
struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
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
magnitude (Sfi::FBlock::iterator i)
{
  return sqrt (*i * *i + *(i+1) * *(i+1));
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
  vector< vector<double> > state_positions;
  vector< vector<double> > state_meanings;

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
#if 0
  double speedup_factor;
  if (options.fundamental_freq > 0)
    speedup_factor = (mix_freq/2048*16) / options.fundamental_freq;
  else
    speedup_factor = 1; /* no resampling */
  MiniResampler resampler (dhandle, speedup_factor);
#endif
  for (uint64 pos = 0; pos < n_values; pos += frame_step)
    {
      AudioBlock audio_block;

      /* read data from file, zeropad last blocks */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);
             //  resampler.read (pos, block.size(), &block[0]);

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

      vector<double> position;
      for (size_t n = 0; n < n_dimensions; n++)
        position.push_back (g_random_double_range (0, 1));

      state_positions.push_back (position);
      state_meanings.push_back (out);

      SfiFBlock *p = sfi_fblock_new_sized (n_dimensions);
      SfiFBlock *m = sfi_fblock_new_sized (block_size * zeropad + 2);

      std::copy (position.begin(), position.end(), p->values);
      std::copy (out.begin(), out.end(), m->values);

      //block.position = *p;
      audio_block.meaning = *m;           // <- will be overwritten by noise spectrum later on
      audio_block.original_fft.resize (m->n_values);
      std::copy (out.begin(), out.end(), audio_block.original_fft.begin());
      if (options.debug)
        {
          audio_block.debug_samples.resize (frame_size);
          std::copy (debug_samples.begin(), debug_samples.begin() + frame_size, audio_block.debug_samples.begin());
        }
      //audio_block.original_fft = *m;
      audio_blocks += audio_block;
    }
  // Track frequencies step #0: find maximum of all values
  double max_mag = 0;
  for (size_t n = 0; n < audio_blocks.length(); n++)
    {
      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
	  max_mag = std::max (max_mag, magnitude (audio_blocks[n]->meaning.begin() + d));
	}
    }

  // Track frequencies step #1: search for local maxima as potential track candidates
  vector< vector<Tracksel> > frame_tracksels (audio_blocks.length()); /* Analog to Canny Algorithms edgels */
  for (size_t n = 0; n < audio_blocks.length(); n++)
    {
      vector<double> mag_values (audio_blocks[n]->meaning.length() / 2);
      for (size_t d = 0; d < block_size * zeropad; d += 2)
        mag_values[d / 2] = magnitude (audio_blocks[n]->meaning.begin() + d);

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
              if (mag2 > -60)
                {
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
                  tracksel.next = 0;
                  tracksel.prev = 0;

                  double dummy_freq;
                  tracksel.is_harmonic = check_harmonic (tracksel.freq, dummy_freq, mix_freq);
                  // FIXME: need a different criterion here
                  // mag2 > -30 doesn't track all partials
                  // mag2 > -60 tracks lots of junk, too
                  if ((mag2 > -60 || tracksel.is_harmonic) && tracksel.freq > 10)
                    frame_tracksels[n].push_back (tracksel);
                }
#if 0
              last_phase[d] = phase;
#endif
            }
	}
    }

  // Track frequencies step #2: link lists together
  for (size_t n = 0; n + 1 < audio_blocks.length(); n++)
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
  for (size_t n = 0; n < audio_blocks.length(); n++)
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
	      if (biggest_mag > -45 || is_harmonic)
		{
		  for (Tracksel *t = &(*i); t->next; t = t->next)
		    {
#if 0
		      double new_freq;
		      if (check_harmonic (t->freq, new_freq, mix_freq))
			t->freq = new_freq;
#endif

		      audio_blocks[t->frame]->freqs.append (t->freq);
		      audio_blocks[t->frame]->phases.append (t->mag);
		      audio_blocks[t->frame]->phases.append (0);
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

  for (uint64 frame = 0; frame < audio_blocks.length(); frame++)
    {
      fill (in.begin(), in.end(), 0);

      // compute spectrum of isolated sine frequencies from audio spectrum
      for (size_t i = 0; i < audio_blocks[frame]->freqs.length(); i++)
	{
	  double phase = 0;
	  for (size_t k = 0; k < block_size; k++)
	    {
	      double freq = *(audio_blocks[frame]->freqs.begin() + i);
	      double re = *(audio_blocks[frame]->phases.begin() + i * 2);
	      double im = *(audio_blocks[frame]->phases.begin() + i * 2 + 1);
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

	  double mag = magnitude (audio_blocks[frame]->meaning.begin() + d);
	  debug ("spectrum:%lld %g\n", frame, mag);
	  if (mag > 0)
	    {
	      *(audio_blocks[frame]->meaning.begin() + d) /= mag;
	      *(audio_blocks[frame]->meaning.begin() + d + 1) /= mag;
	      mag -= sub_mag;
	      if (mag < 0)
		mag = 0;
	      *(audio_blocks[frame]->meaning.begin() + d) *= mag;
	      *(audio_blocks[frame]->meaning.begin() + d + 1) *= mag;
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

  for (uint64 frame = 0; frame < audio_blocks.length(); frame++)
    {
      vector<double> noise_envelope (256);
      vector<double> spectrum (audio_blocks[frame]->meaning.begin(), audio_blocks[frame]->meaning.end());

      approximate_noise_spectrum (frame, spectrum, noise_envelope);

      vector<double> approx_spectrum (2050);
      xnoise_envelope_to_spectrum (noise_envelope, approx_spectrum);
      for (int i = 0; i < 2048; i += 2)
	debug ("spect_approx:%lld %g\n", frame, approx_spectrum[i]);
      audio_blocks[frame]->meaning.resize (noise_envelope.size());
      copy (noise_envelope.begin(), noise_envelope.end(), audio_blocks[frame]->meaning.begin());
    }

  codebook.push_back (vector<double> (block_size + 2));
  codebook_centroid.push_back (vector<double> (block_size + 2));
  if (options.quantize_entries)
    {
      for (size_t CBS = 0; CBS < std::min (options.quantize_entries, audio_blocks.length()); CBS++)
	{
	  fprintf (stderr, "%zd", CBS);
	  double max_adj = 1;
	  while (max_adj > 1e-10)
	    {
	      max_adj = 0;
	      for (size_t k = 0; k < codebook.size(); k++)
		{
		  codebook_usage[k] = 0;
		  std::fill (codebook_centroid[k].begin(), codebook_centroid[k].end(), 0);
		}
	      for (size_t n = 0; n < audio_blocks.length(); n++)
		{
		  size_t dist_k = search_nearest_codebook_entry (n);
		  codebook_usage[dist_k]++;
		  for (size_t i = 0; i < block_size + 2; i++)
		    codebook_centroid[dist_k][i] += *(audio_blocks[n]->meaning.begin() + i);
		}
	      biggest_usage = 0;
	      biggest_usage_k = -1;
	      int ch = 0;
	      for (size_t k = 0; k < codebook.size(); k++)
		{
		  double adj = 0;
		  for (size_t i = 0; i < block_size + 2; i++)
		    {
		      codebook_centroid[k][i] /= codebook_usage[k];
		      double a = codebook_centroid[k][i] - codebook[k][i];
		      adj += a * a;
		    }
		  if (codebook[k] != codebook_centroid[k])
		    {
		      ch++;
		    }
		  codebook[k] = codebook_centroid[k];
		  if (codebook_usage[k] > biggest_usage)
		    {
		      biggest_usage = codebook_usage[k];
		      biggest_usage_k = k;
		    }
		  if (adj > max_adj)
		    max_adj = adj;
		  //printf ("adjusted codebook centroid k=%d adj=%f\n", k, adj);
		}
	      //printf ("biggest_usage = %d\n", biggest_usage);
	      //printf ("biggest_usage_k = %d\n", biggest_usage_k);
	      //printf ("(%d/%d) changes\n", ch, codebook.size());
	    }
	  vector<double> codebook_error (codebook.size());
	  for (size_t n = 0; n < audio_blocks.length(); n++)
	    {
	      size_t k = search_nearest_codebook_entry (n);
	      for (size_t i = 0; i < block_size + 2; i++)
		{
		  double error = codebook[k][i] - *(audio_blocks[n]->meaning.begin() + i);
		  codebook_error[k] += error * error;
		}
	    }
	  size_t biggest_error_k = 0;
	  double biggest_error = 0.0, error_sum = 0;
	  for (size_t k = 0; k < codebook.size(); k++)
	    {
	      if (codebook_error[k] > biggest_error)
		{
		  biggest_error = codebook_error[k];
		  biggest_error_k = k;
		}
	      error_sum += codebook_error[k];
	    }
	  fprintf (stderr, " %f errorDB\r", 10 * log10 (error_sum / block_size / audio_blocks.length()));
	  //printf ("*** %f\n", error_sum);
	  /* figure out 2 biggest_usage_k representants */
	  vector<size_t> cbadd;
	  while (cbadd.size() != 2)
	    {
	      size_t n = rand() % audio_blocks.length();
	      size_t dist_k = search_nearest_codebook_entry (n);
	      if (dist_k == biggest_error_k)
		{
		  if (cbadd.empty())
		    {
		      cbadd.push_back (n);
		    }
		  else if (cbadd[0] != n) // cbadd.size() == 1
		    {
		      /* use second vector only if it differs from the first,
		       * since using twice the same vector will not actually
		       * split the cluster
		       */
		      if (!std::equal (audio_blocks[cbadd[0]]->meaning.begin(),
				       audio_blocks[cbadd[0]]->meaning.end(),
				       audio_blocks[n]->meaning.begin()))
			cbadd.push_back (n);
		    }
		}
	    }
	  //printf ("%d %d\n", cbadd[0], cbadd[1]);
	  Sfi::FBlock& b = audio_blocks[cbadd[0]]->meaning;
	  std::copy (b.begin(), b.end(), codebook[biggest_usage_k].begin());
	  codebook_centroid[biggest_error_k] = vector<double> (block_size + 2);
	  codebook.push_back (vector<double> (block_size + 2));
	  codebook_centroid.push_back (vector<double> (block_size + 2));
	  Sfi::FBlock& b1 = audio_blocks[cbadd[1]]->meaning;
	  std::copy (b1.begin(), b1.end(), codebook[codebook.size() - 1].begin());
	  //printf ("%d\n", search_nearest_codebook_entry (cbadd[0]));
	  //printf ("%d\n", search_nearest_codebook_entry (cbadd[1]));
	}

      for (size_t n = 0; n < audio_blocks.length(); n++)
        {
	  size_t k = search_nearest_codebook_entry (n);
	  std::copy (codebook[k].begin(), codebook[k].end(), audio_blocks[n]->meaning.begin());
	}
    }
  Stw::Codec::Audio audio;
  audio.fundamental_freq = options.fundamental_freq;
  audio.mix_freq = enc_params.mix_freq;
  audio.frame_size_ms = enc_params.frame_size_ms;
  audio.frame_step_ms = enc_params.frame_step_ms;
  audio.zeropad = zeropad;
  audio.contents = audio_blocks;
  STWAFile::save (argv[2], audio);
}
