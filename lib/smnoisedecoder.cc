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

#include "smnoisedecoder.hh"
#include "smfft.hh"
#include <bse/gslfft.h>
#include <bse/bsemathsignal.h>
#include <stdio.h>
#include <math.h>

using std::vector;
using SpectMorph::NoiseDecoder;
using SpectMorph::Frame;

/**
 * Creates a noise decoder object.
 *
 * \param orig_mix_freq   original mix freq (sample rate) of the audio file that was encoded
 * \param mix_freq        mix freq (sample rate) of the output sample data
 */
NoiseDecoder::NoiseDecoder (double orig_mix_freq, double mix_freq) :
  orig_mix_freq (orig_mix_freq),
  mix_freq (mix_freq)
{
}

void
NoiseDecoder::set_seed (int seed)
{
  random_gen.set_seed (seed);
}

void
NoiseDecoder::noise_envelope_to_spectrum (const vector<double>& envelope,
			                  float *spectrum, size_t spectrum_size)
{
  for (size_t d = 0; d < spectrum_size; d += 2)
    {
      double freq = (d * mix_freq * 0.5) / spectrum_size;
      double pos = freq / orig_mix_freq * envelope.size() * 2;
      size_t ipos = pos;
      if (ipos + 1 < envelope.size())
        {
          double f = pos - ipos;
	  spectrum[d] = bse_db_to_factor (envelope[ipos] * (1 - f)
                                        + envelope[ipos + 1] * f);
        }
      else
        {
          // else: outside envelope
          spectrum[d] = 0;
        }
      spectrum[d+1] = 0;
    }
#if 0
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
      //debug ("noiseint %f\n", spectrum[d]);
    }
#endif
}

static size_t
next_power2 (size_t i)
{
  size_t p = 1;
  while (p < i)
    p *= 2;
  return p;
}

void
xnoise_envelope_to_spectrum (double mix_freq,
                             const vector<double>& envelope,
			     vector<double>& spectrum);

/**
 * This function decodes the noise contained in the frame and
 * fills the decoded_residue vector of the frame.
 *
 * \param frame   frame to be decoded; also: location of the output sample data
 * \param window  window function to be used; should be the same or similar to the one used in encoding
 */
void
NoiseDecoder::process (const Frame& frame,
		       const vector<double>& window,
                       vector<float>& decoded_residue)
{
  const size_t block_size = next_power2 (decoded_residue.size());

  vector<double> dinterpolated_spectrum (block_size + 2);
  xnoise_envelope_to_spectrum (mix_freq, frame.noise_envelope, dinterpolated_spectrum);

  const double Eww = 0.375;
  const double norm = block_size * block_size * 0.5 / Eww;

  float *interpolated_spectrum = FFT::new_array_float (block_size + 2);
  for (size_t i = 0; i < block_size; i += 2)
    {
      double phase = random_gen.random_double_range (0, 2 * M_PI);
      double a = sin (phase);
      double b = cos (phase);
      interpolated_spectrum[i] = a * dinterpolated_spectrum[i] * sqrt (norm);
      interpolated_spectrum[i+1] = b * dinterpolated_spectrum[i] * sqrt (norm);
      //debug ("noise:%lld %f %f\n", pos * overlap / block_size, interpolated_spectrum[i], interpolated_spectrum[i+1]);
    }
  interpolated_spectrum[1] = interpolated_spectrum[block_size];
  float *in = FFT::new_array_float (block_size);
  FFT::fftsr_float (block_size, &interpolated_spectrum[0], &in[0]);

  double r_energy = 0;
  for (size_t i = 0; i < decoded_residue.size(); i++)
    {
      // double windowing will allow phase modifications
      decoded_residue[i] = in[i] * window[i];
      //debug ("out:%lld %f\n", pos * overlap / block_size, out_sample[i]);
      r_energy += decoded_residue[i] * decoded_residue[i];

      // compensate for overlap
      decoded_residue[i] /= 2;
    }

#if 0 // DEBUG
  r_energy /= decoded_residue.size();
  double s_energy = 0;
  for (size_t i = 0; i < dinterpolated_spectrum.size(); i++)
    {
      double d = dinterpolated_spectrum[i];
      s_energy += d * d;
    }

  double xs_energy = 0;
  for (vector<double>::const_iterator fni = frame.noise_envelope.begin(); fni != frame.noise_envelope.end(); fni++)
    xs_energy += *fni;

  double i_energy = 0;
  for (size_t i = 0; i < block_size; i++)
    i_energy += interpolated_spectrum[i] * interpolated_spectrum[i] / norm;
  printf ("RE %f SE %f XE %f IE %f\n", r_energy, s_energy, xs_energy, i_energy);
#endif

  FFT::free_array_float (in);
  FFT::free_array_float (interpolated_spectrum);
}


