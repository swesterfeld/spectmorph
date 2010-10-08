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
#include "smmath.hh"
#include "smfft.hh"
#include <bse/bseblockutils.hh>
#include <bse/gslfft.h>
#include <bse/bsemathsignal.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <map>

using std::vector;
using SpectMorph::NoiseDecoder;
using SpectMorph::Frame;
using std::map;

static map<size_t, float *> cos_window_for_block_size;

static size_t
next_power2 (size_t i)
{
  size_t p = 1;
  while (p < i)
    p *= 2;
  return p;
}

/**
 * Creates a noise decoder object.
 *
 * \param orig_mix_freq   original mix freq (sample rate) of the audio file that was encoded
 * \param mix_freq        mix freq (sample rate) of the output sample data
 */
NoiseDecoder::NoiseDecoder (double orig_mix_freq, double mix_freq, size_t block_size) :
  orig_mix_freq (orig_mix_freq),
  mix_freq (mix_freq),
  block_size (block_size)
{
  noise_band_partition = 0;

  float*& win = cos_window_for_block_size[block_size];
  if (!win)
    {
      win = FFT::new_array_float (block_size);
      for (size_t i = 0; i < block_size; i++)
        win[i] = bse_window_cos (2.0 * i / block_size - 1.0);
    }

  cos_window = win;

  assert (block_size == next_power2 (block_size));
}

NoiseDecoder::~NoiseDecoder()
{
  if (noise_band_partition)
    {
      delete noise_band_partition;
      noise_band_partition = 0;
    }
}

void
NoiseDecoder::set_seed (int seed)
{
  random_gen.set_seed (seed);
}

/**
 * This function decodes the noise contained in the frame and
 * fills the decoded_residue vector of the frame.
 *
 * \param audio_block   AudioBlock to be decoded
 */
void
NoiseDecoder::process (const AudioBlock& audio_block,
                       float            *samples,
                       OutputMode        output_mode)
{
  if (!noise_band_partition)
    noise_band_partition = new NoiseBandPartition (audio_block.noise.size(), block_size + 2, mix_freq);

  assert (noise_band_partition->n_bands() == audio_block.noise.size());
  assert (noise_band_partition->n_spectrum_bins() == block_size + 2);

  float *interpolated_spectrum = FFT::new_array_float (block_size + 2);

  const double Eww = 0.375;
  const double norm = block_size * block_size / Eww;

  noise_band_partition->noise_envelope_to_spectrum (random_gen, audio_block.noise, interpolated_spectrum, sqrt (norm) / 2);

  interpolated_spectrum[1] = interpolated_spectrum[block_size];
  float *in = FFT::new_array_float (block_size);
  FFT::fftsr_float (block_size, &interpolated_spectrum[0], &in[0]);

  Bse::Block::mul (block_size, in, cos_window);

  if (output_mode == REPLACE)
    memcpy (samples, in, block_size * sizeof (float));
  else if (output_mode == ADD)
    Bse::Block::add (block_size, samples, in);
  else
    assert (false);

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

size_t
NoiseDecoder::preferred_block_size (double mix_freq)
{
  size_t bs = 1;

  while (bs * 2 / mix_freq < 0.040)    /* block size should not exceed 40ms (and be power of 2) */
    bs *= 2;

  return bs;
}
