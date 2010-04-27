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
#include "smafile.hh"

#include <bse/bsemathsignal.h>
#include <bse/gslfft.h>
#include <math.h>

#include <complex>
#include <map>

using SpectMorph::Encoder;
using std::vector;
using std::string;
using std::map;
using std::max;

double
magnitude (vector<float>::iterator i)
{
  return sqrt (*i * *i + *(i+1) * *(i+1));
}

void
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


void
Encoder::search_local_maxima (vector< vector<Tracksel> >& frame_tracksels)
{
  const size_t block_size = enc_params.block_size;
  const size_t frame_size = enc_params.frame_size;
  const int    zeropad    = enc_params.zeropad;
  const double mix_freq   = enc_params.mix_freq;

  // find maximum of all values
  double max_mag = 0;
  for (size_t n = 0; n < audio_blocks.size(); n++)
    {
      for (size_t d = 2; d < block_size * zeropad; d += 2)
	{
	  max_mag = std::max (max_mag, magnitude (audio_blocks[n].meaning.begin() + d));
	}
    }


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
}

void
Encoder::link_partials (vector< vector<Tracksel> >& frame_tracksels)
{
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
}

void
Encoder::validate_partials (vector< vector<Tracksel> >& frame_tracksels)
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
		      audio_blocks[t->frame].phases.push_back (t->phaseb);
		      audio_blocks[t->frame].phases.push_back (t->phasea);
		    }
		}
	    }
	}
    }
}

void
Encoder::spectral_subtract (const vector<float>& window)
{
  const size_t block_size = enc_params.block_size;
  const size_t zeropad = enc_params.zeropad;

  for (uint64 frame = 0; frame < audio_blocks.size(); frame++)
    {
      vector<double> in (block_size * zeropad), out (block_size * zeropad + 2);

      // compute spectrum of isolated sine frequencies from audio spectrum
      for (size_t i = 0; i < audio_blocks[frame].freqs.size(); i++)
	{
	  double phase = 0;
	  for (size_t k = 0; k < enc_params.block_size; k++)
	    {
	      double freq = audio_blocks[frame].freqs[i];
	      double re = audio_blocks[frame].phases[i * 2];
	      double im = audio_blocks[frame].phases[i * 2 + 1];
	      double mag = sqrt (re * re + im * im);
	      phase += freq / enc_params.mix_freq * 2 * M_PI;
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
Encoder::approx_noise()
{
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
}

void
Encoder::save (const string& filename, double fundamental_freq)
{
  SpectMorph::Audio audio;
  audio.fundamental_freq = fundamental_freq;
  audio.mix_freq = enc_params.mix_freq;
  audio.frame_size_ms = enc_params.frame_size_ms;
  audio.frame_step_ms = enc_params.frame_step_ms;
  audio.zeropad = enc_params.zeropad;
  audio.contents = audio_blocks;
  STWAFile::save (filename, audio);
}


