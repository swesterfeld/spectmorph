/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smencoder.hh"
#include "smmath.hh"

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/bindings/lapack/lapack.hpp>

#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>
#include <bse/gslfft.h>
#include <math.h>
#include <stdio.h>

#include <complex>
#include <map>

using SpectMorph::Encoder;
using SpectMorph::AudioBlock;
using SpectMorph::Tracksel;
using SpectMorph::VectorSinParams;
using Birnet::AlignedArray;
using std::vector;
using std::string;
using std::map;
using std::max;

namespace ublas = boost::numeric::ublas;
using ublas::matrix;

static double
magnitude (vector<float>::iterator i)
{
  return sqrt (*i * *i + *(i+1) * *(i+1));
}

static void
debug (const char *dbg, ...)
{
  va_list ap;

  // FIXME!
  return;
#if 0
  if (!options.debug)
    return;

  va_start (ap, dbg);
  vfprintf (options.debug, dbg, ap);
  va_end (ap);
#endif
}

/**
 * Constructor which initializes the Encoders parameters.
 */
Encoder::Encoder (const EncoderParams& enc_params)
{
  this->enc_params = enc_params;
  optimal_attack.attack_start_ms = 0;
  optimal_attack.attack_end_ms = 0;
}

bool
Encoder::check_harmonic (double freq, double& new_freq, double mix_freq)
{
  if (enc_params.fundamental_freq > 0)
    {
      for (int i = 1; i < 100; i++)
	{
          // FIXME: why hardcode 2048
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

/**
 * This function computes the short-time-fourier-transform (STFT) of the input
 * signal using a window to cut the individual frames out of the sample.
 */
void
Encoder::compute_stft (GslDataHandle *dhandle, const vector<float>& window)
{
  const uint64 n_values = gsl_data_handle_length (dhandle);
  const size_t frame_size = enc_params.frame_size;
  const size_t block_size = enc_params.block_size;
  const int    zeropad    = enc_params.zeropad;

  vector<float> block (block_size);
  vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);

  for (uint64 pos = 0; pos < n_values; pos += enc_params.frame_step)
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
      Bse::Block::mul (enc_params.block_size, &block[0], &window[0]);

      int j = in.size() - enc_params.frame_size / 2;
      for (vector<float>::const_iterator i = block.begin(); i != block.end(); i++)
        in[(j++) % in.size()] = *i;

      gsl_power2_fftar (block_size * zeropad, &in[0], &out[0]);
      out[block_size * zeropad] = out[1];
      out[block_size * zeropad + 1] = 0;
      out[1] = 0;

      audio_block.noise.assign (out.begin(), out.end()); // <- will be overwritten by noise spectrum later on
      audio_block.original_fft.assign (out.begin(), out.end());
      audio_block.debug_samples.assign (debug_samples.begin(), debug_samples.begin() + frame_size);

      audio_blocks.push_back (audio_block);
    }
}

/**
 * This function searches for peaks in the frame ffts. These are stored in frame_tracksels.
 */
void
Encoder::search_local_maxima()
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const int    zeropad    = enc_params.zeropad;
  const double mix_freq   = enc_params.mix_freq;

  // initialize tracksel structure
  frame_tracksels.clear();
  frame_tracksels.resize (audio_blocks.size());

  // find maximum of all values
  double max_mag = 0;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
	  max_mag = std::max (max_mag, magnitude (audio_blocks[n].noise.begin() + d));
	}
    }

  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      vector<double> mag_values (audio_blocks[n].noise.size() / 2);
      for (size_t d = 0; d < block_size * zeropad; d += 2)
        mag_values[d / 2] = magnitude (audio_blocks[n].noise.begin() + d);

      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
#if 0
	  double phase = atan2 (*(audio_blocks[n]->noise.begin() + d),
	                        *(audio_blocks[n]->noise.begin() + d + 1)) / 2 / M_PI;  /* range [-0.5 .. 0.5] */
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
                  for (de = d / 2 + 1; de < (mag_values.size() - 1) && mag_values[de] > mag_values[de + 1]; de++);
                  if (de - ds > 12)
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

                      // use the interpolation formula for the complex values to find the phase
                      std::complex<double> c1 (audio_blocks[n].noise[d-2], audio_blocks[n].noise[d-1]);
                      std::complex<double> c2 (audio_blocks[n].noise[d], audio_blocks[n].noise[d+1]);
                      std::complex<double> c3 (audio_blocks[n].noise[d+2], audio_blocks[n].noise[d+3]);
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
                      tracksel.cmag = interp_c.real() / frame_size * zeropad;
                      tracksel.smag = interp_c.imag() / frame_size * zeropad;
                      tracksel.next = 0;
                      tracksel.prev = 0;

                      // correct for the odd-centered analysis
                        {
                          double smag = tracksel.smag;
                          double cmag = tracksel.cmag;
                          double magnitude = sqrt (smag * smag + cmag * cmag);
                          double phase = atan2 (smag, cmag);
                          phase += (frame_size - 1) / 2.0 / mix_freq * tracksel.freq * 2 * M_PI;
                          smag = sin (phase) * magnitude;
                          cmag = cos (phase) * magnitude;
                          tracksel.smag = smag;
                          tracksel.cmag = cmag;
                        }


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
}

/// @cond
struct
PeakIndex
{
  double                      freq;
  vector<Tracksel>::iterator  i;
  PeakIndex                  *prev;
  double                      prev_delta;

  PeakIndex (double freq, vector<Tracksel>::iterator i)
    : freq (freq), i (i), prev (0), prev_delta (0)
  {
  }
};
/// @endcond

static bool
partial_index_cmp (const PeakIndex& a, const PeakIndex& b)
{
  return a.freq < b.freq;
}

/**
 * This function links the spectral peaks (contained in the Tracksel structure)
 * of successive frames together by setting the prev and next pointers. It
 * tries to minimize the frequency difference between the peaks that are linked
 * together, while using a threshold of 5% frequency derivation.
 */
void
Encoder::link_partials()
{
  for (size_t n = 0; n + 1 < audio_blocks.size(); n++)
    {
      // build sorted index for this frame
      vector<PeakIndex> current_index;
      for (vector<Tracksel>::iterator i = frame_tracksels[n].begin(); i != frame_tracksels[n].end(); i++)
        current_index.push_back (PeakIndex (i->freq, i));
      sort (current_index.begin(), current_index.end(), partial_index_cmp);

      // build sorted index for next frame
      vector<PeakIndex> next_index;
      for (vector<Tracksel>::iterator i = frame_tracksels[n + 1].begin(); i != frame_tracksels[n + 1].end(); i++)
        next_index.push_back (PeakIndex (i->freq, i));
      sort (next_index.begin(), next_index.end(), partial_index_cmp);

      vector<PeakIndex>::iterator ci = current_index.begin();
      vector<PeakIndex>::iterator ni = next_index.begin();
      if (ni != next_index.end())    // if current or next frame are empty (no peaks) there is nothing to do
        {
          while (ci != current_index.end())
            {
              /*
               * increment ni as long as incrementing it makes ni point to a
               * better (closer) peak below ci's frequency
               */
              vector<PeakIndex>::iterator inc_ni;
              do
                {
                  inc_ni = ni + 1;
                  if (inc_ni < next_index.end() && inc_ni->freq < ci->freq)
                    ni = inc_ni;
                }
              while (ni == inc_ni);

              /*
               * possible candidates for a match are
               * - ni      - which contains the greatest peak with a smaller frequency than ci->freq
               * - ni + 1  - which contains the smallest peak with a greater frequency that ci->freq
               * => choose the candidate which is closer to ci->freq
               */
              vector<PeakIndex>::iterator besti = ni;
              if (ni + 1 < next_index.end() && fabs (ci->freq - (ni + 1)->freq) < fabs (ci->freq - ni->freq))
                besti = ni + 1;

              const double delta = fabs (ci->freq - besti->freq) / ci->freq;
              if (delta < 0.05) /* less than 5% frequency derivation */
                {
                  if (!besti->prev || besti->prev_delta > delta)
                    {
                      besti->prev = &(*ci);
                      besti->prev_delta = delta;
                    }
                }
              ci++;
            }

          /* link best matches (with the smallest frequency derivation) */
          for (ni = next_index.begin(); ni != next_index.end(); ni++)
            {
              if (ni->prev)
                {
                  Tracksel *crosslink_a = &(*ni->prev->i);
                  Tracksel *crosslink_b = &(*ni->i);
                  crosslink_a->next = crosslink_b;
                  crosslink_b->prev = crosslink_a;
                }
            }
        }
    }
}

/**
 * This function validates that the partials found by the peak linking have
 * good quality.
 */
void
Encoder::validate_partials()
{
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
		      audio_blocks[t->frame].phases.push_back (t->smag);
		      audio_blocks[t->frame].phases.push_back (t->cmag);
		    }
		}
	    }
	}
    }
}

/**
 * This function subtracts the partials from the audio signal, to get the
 * residue (remaining energy not corresponding to sine frequencies).
 */
void
Encoder::spectral_subtract (const vector<float>& window)
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const size_t zeropad = enc_params.zeropad;

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      AlignedArray<float,16> signal (frame_size);
      for (size_t i = 0; i < audio_blocks[frame].freqs.size(); i++)
	{
          const double freq = audio_blocks[frame].freqs[i];
	  const double smag = audio_blocks[frame].phases[i * 2];
	  const double cmag = audio_blocks[frame].phases[i * 2 + 1];

          VectorSinParams params;
          params.mix_freq = enc_params.mix_freq;
          params.freq = freq;
          params.phase = atan2 (cmag, smag);
          params.mag = sqrt (smag * smag + cmag * cmag);
          params.mode = VectorSinParams::ADD;

          fast_vector_sinf (params, &signal[0], &signal[frame_size]);
	}
      vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);
      // apply window
      for (size_t k = 0; k < enc_params.block_size; k++)
        in[k] = window[k] * signal[k];
      // FFT
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

	  double mag = magnitude (audio_blocks[frame].noise.begin() + d);
	  debug ("spectrum:%lld %g\n", frame, mag);
	  if (mag > 0)
	    {
	      audio_blocks[frame].noise[d] /= mag;
	      audio_blocks[frame].noise[d + 1] /= mag;
	      mag -= sub_mag;
	      if (mag < 0)
		mag = 0;
	      audio_blocks[frame].noise[d] *= mag;
	      audio_blocks[frame].noise[d + 1] *= mag;
	    }
	  debug ("finalspectrum:%lld %g\n", frame, mag);
	}
    }
}

// find best fit of amplitudes / phases to the observed signal
static void
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

template<class AIter, class BIter>
static double
float_vector_delta (AIter ai, AIter aend, BIter bi)
{
  double d = 0;
  while (ai != aend)
    {
      double dd = *ai++ - *bi++;
      d += dd * dd;
    }
  return d;
}

static void
refine_sine_params_fast (AudioBlock& audio_block, double mix_freq, int frame, const vector<float>& window)
{
  const size_t frame_size = audio_block.debug_samples.size();

  AlignedArray<float, 16> sin_vec (frame_size);
  AlignedArray<float, 16> cos_vec (frame_size);
  AlignedArray<float, 16> sines (frame_size);
  vector<float> good_freqs;
  vector<float> good_phases;

  // delta against null vector
  double delta = float_vector_delta (&sines[0], &sines[frame_size], audio_block.debug_samples.begin());
  double max_mag;
  size_t partial = 0;
  do
    {
      max_mag = 0;
      // search biggest partial
      for (size_t i = 0; i < audio_block.freqs.size(); i++)
        {
          const double smag = audio_block.phases[2 * i];
          const double cmag = audio_block.phases[2 * i + 1];
          const double mag = sqrt (smag * smag + cmag * cmag);

          if (mag > max_mag)
            {
              partial = i;
              max_mag = mag;
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

            VectorSinParams params;

            params.mix_freq = mix_freq;
            params.freq = f;
            params.mag = 1;
            params.phase = -((frame_size - 1) / 2.0) * f / mix_freq * 2.0 * M_PI;
            while (params.phase < -M_PI)
              params.phase += 2 * M_PI;
            params.mode = VectorSinParams::REPLACE;

            fast_vector_sincosf (params, &sin_vec[0], &sin_vec[frame_size], &cos_vec[0]);

            for (size_t n = 0; n < frame_size; n++)
              {
                const double v = audio_block.debug_samples[n] - sines[n];
                const double swin = sin_vec[n] * window[n];
                const double cwin = cos_vec[n] * window[n];

                smag += v * window[n] * swin;
                cmag += v * window[n] * cwin;
                snorm += swin * swin;
                cnorm += cwin * cwin;
              }
            smag /= snorm;
            cmag /= cnorm;

            double magnitude = sqrt (smag * smag + cmag * cmag);
            phase = atan2 (smag, cmag);
            phase += (frame_size - 1) / 2.0 / mix_freq * f * 2 * M_PI;
            smag = sin (phase) * magnitude;
            cmag = cos (phase) * magnitude;

            vector<float> old_sines (&sines[0], &sines[frame_size]);

            // restore partial => sines; keep params.freq & params.mix_freq
            params.phase = atan2 (cmag, smag);
            params.mag = sqrt (smag * smag + cmag * cmag);
            params.mode = VectorSinParams::ADD;
            fast_vector_sinf (params, &sines[0], &sines[frame_size]);

            double new_delta = float_vector_delta (&sines[0], &sines[frame_size], audio_block.debug_samples.begin());
            if (new_delta >= delta)      // approximation is _not_ better
              {
                std::copy (old_sines.begin(), old_sines.end(), &sines[0]);
              }
            else
              {
                delta = new_delta;
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

/**
 * This function reestimates the magnitudes and phases of the partials found
 * in the previous steps.
 */
void
Encoder::optimize_partials (const vector<float>& window, int optimization_level)
{
  const double mix_freq = enc_params.mix_freq;

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      if (optimization_level >= 1) // redo FFT estmates, only better
        refine_sine_params_fast (audio_blocks[frame], mix_freq, frame, window);

      if (optimization_level >= 2)
        {
          // do "perfect" magnitude and phase estimation using linear least squares
          refine_sine_params (audio_blocks[frame], mix_freq, window);
          printf ("refine: %2.3f %%\r", frame * 100.0 / audio_blocks.size());
          fflush (stdout);
        }
      remove_small_partials (audio_blocks[frame]);
    }
}

static void
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

static void
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

/**
 * This function tries to approximate the residual by a spectral envelope
 * for a noise signal.
 */
void
Encoder::approx_noise()
{
  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      vector<double> noise_envelope (256);
      vector<double> spectrum (audio_blocks[frame].noise.begin(), audio_blocks[frame].noise.end());

      approximate_noise_spectrum (frame, spectrum, noise_envelope);

      vector<double> approx_spectrum (2050);
      xnoise_envelope_to_spectrum (noise_envelope, approx_spectrum);
      for (int i = 0; i < 2048; i += 2)
	debug ("spect_approx:%lld %g\n", frame, approx_spectrum[i]);
      audio_blocks[frame].noise.resize (noise_envelope.size());
      copy (noise_envelope.begin(), noise_envelope.end(), audio_blocks[frame].noise.begin());
    }
}

double
Encoder::attack_error (const vector< vector<double> >& unscaled_signal, const Attack& attack, vector<double>& out_scale)
{
  const size_t frames = unscaled_signal.size();
  double total_error = 0;

  for (size_t f = 0; f < frames; f++)
    {
      const vector<double>& frame_signal = unscaled_signal[f];
      size_t zero_values = 0;
      double scale;

      for (size_t n = 0; n < frame_signal.size(); n++)
        {
          const double n_ms = f * enc_params.frame_step_ms + n * 1000.0 / enc_params.mix_freq;
          double env;
          scale = (zero_values > 0) ? frame_signal.size() / double (frame_signal.size() - zero_values) : 1.0;
          if (n_ms < attack.attack_start_ms)
            {
              env = 0;
              zero_values++;
            }
          else if (n_ms < attack.attack_end_ms)  // during attack
            {
              const double attack_len_ms = attack.attack_end_ms - attack.attack_start_ms;

              env = (n_ms - attack.attack_start_ms) / attack_len_ms;
            }
          else // after attack
            {
              env = 1.0;
            }
          const double value = frame_signal[n] * scale * env;
          const double error = value - audio_blocks[f].debug_samples[n];
          total_error += error * error;
        }
      out_scale[f] = scale;
    }
  return total_error;
}

void
Encoder::compute_attack_params()
{
  const double mix_freq   = enc_params.mix_freq;
  const size_t frame_size = enc_params.frame_size;
  const size_t frames = MIN (20, audio_blocks.size());

  vector< vector<double> > unscaled_signal;
  for (size_t f = 0; f < frames; f++)
    {
      const AudioBlock& audio_block = audio_blocks[f];
      vector<double> frame_signal (frame_size);

      for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
        {
          double smag = audio_block.phases[2 * partial];
          double cmag = audio_block.phases[2 * partial + 1];
          double f    = audio_block.freqs[partial];
          double phase = 0;

          // do a phase optimal reconstruction of that partial
          for (size_t n = 0; n < frame_signal.size(); n++)
            {
              frame_signal[n] += sin (phase) * smag;
              frame_signal[n] += cos (phase) * cmag;
              phase += f / mix_freq * 2.0 * M_PI;
            }
        }
      unscaled_signal.push_back (frame_signal);
    }

  Attack attack;
  int no_modification = 0;
  double error = 1e7;
  vector<double> scale (frames);

  attack.attack_start_ms = 0;
  attack.attack_end_ms = 10;
  while (no_modification < 3000)
    {
      double R;
      Attack new_attack = attack;
      if (no_modification < 500)
        R = 100;
      else if (no_modification < 1000)
        R = 20;
      else if (no_modification < 1500)
        R = 1;
      else if (no_modification < 2000)
        R = 0.2;
      else if (no_modification < 2500)
        R = 0.01;

      new_attack.attack_start_ms += g_random_double_range (-R, R);
      new_attack.attack_end_ms += g_random_double_range (-R, R);

      if (new_attack.attack_start_ms < new_attack.attack_end_ms &&
          new_attack.attack_start_ms >= 0 &&
          new_attack.attack_end_ms < 200)
        {
          const double new_error = attack_error (unscaled_signal, new_attack, scale);
#if 0
          printf ("attack=<%f, %f> error=%.17g new_attack=<%f, %f> new_arror=%.17g\n", attack.attack_start_ms, attack.attack_end_ms, error,
                                                                                       new_attack.attack_start_ms, new_attack.attack_end_ms, new_error);
#endif
          if (new_error < error)
            {
              error = new_error;
              attack = new_attack;

              no_modification = 0;
            }
          else
            {
              no_modification++;
            }
        }
    }
  for (size_t f = 0; f < frames; f++)
    {
      for (size_t i = 0; i < audio_blocks[f].phases.size(); i++)
        audio_blocks[f].phases[i] *= scale[f];
    }
  optimal_attack = attack;
}

/**
 * This function saves the data produced by the encoder to a SpectMorph file.
 */
void
Encoder::save (const string& filename, double fundamental_freq)
{
  SpectMorph::Audio audio;
  audio.fundamental_freq = fundamental_freq;
  audio.mix_freq = enc_params.mix_freq;
  audio.frame_size_ms = enc_params.frame_size_ms;
  audio.frame_step_ms = enc_params.frame_step_ms;
  audio.attack_start_ms = optimal_attack.attack_start_ms;
  audio.attack_end_ms = optimal_attack.attack_end_ms;
  audio.zeropad = enc_params.zeropad;
  audio.contents = audio_blocks;
  audio.save (filename);
}




/**
 * \mainpage SpectMorph Index Page
 *
 * \section intro_sec Introduction
 *
 * SpectMorph is a software which analyzes wav files and builds a frame based model of these wav files,
 * where each frame contains information about the spectrum. Each frame is represented as a sum of sine
 * waves, and a noise component. There are command line tools like smenc and smplay for encoding and
 * decoding SpectMorph models, which are documented in the manual pages. Technically, these tools are frontends
 * to the C++ classes in libspectmorph, which are documented here.
 *
 * \section enc_sec Encoding, loading and saving
 *
 * The encoder is implemented in SpectMorph::Encoder. It can be used to encode an audio file; the frames
 * (short snippets of the audio file, maybe 40 ms or so) are then available as vector containing
 * SpectMorph::AudioBlock objects.
 *
 * Many SpectMorph::AudioBlock objects are needed to represent a whole sound file, and the SpectMorph::Audio
 * class is used to store all parameters of an encoded file, along with the actual frames. Information like
 * the sampling rate (mix_freq) or the original note (fundamental freq) are stored in the SpectMorph::Audio
 * class. Functions for storing and loading SpectMorph::Audio objects exist in namespace SpectMorph::AudioFile.
 *
 * \section dec_sec Decoding
 *
 * The decoding process is frame oriented. For each SpectMorph::AudioBlock, a SpectMorph::Frame object needs
 * to be created. For each frame, a SpectMorph::SineDecoder and a SpectMorph::NoiseDecoder can be used to
 * reconstruct (something which sounds like) the original signal.
 */
