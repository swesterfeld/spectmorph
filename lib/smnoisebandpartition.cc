// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <bse/bseblockutils.hh>

#include "smnoisebandpartition.hh"
#include "smmath.hh"

using namespace SpectMorph;
using std::vector;

static double
mel_to_hz (double mel)
{
  return 700 * (exp (mel / 1127.0) - 1);
}

NoiseBandPartition::NoiseBandPartition (size_t n_bands, size_t n_spectrum_bins, double mix_freq) :
  band_count (n_bands),
  band_start (n_bands),
  spectrum_size (n_spectrum_bins)
{
  size_t d = 0;
  /* assign each d to a band */
  vector<int> band_from_d (n_spectrum_bins);
  std::fill (band_from_d.begin(), band_from_d.end(), -1);
  for (size_t band = 0; band < n_bands; band++)
    {
      double mel_low = 30 + 4000.0 / n_bands * band;
      double mel_high = 30 + 4000.0 / n_bands * (band + 1);
      double hz_low = mel_to_hz (mel_low);
      double hz_high = mel_to_hz (mel_high);

      /* skip frequencies which are too low to be in lowest band */
      double f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
      if (band == 0)
        {
          while (f_hz < hz_low)
            {
              d += 2;
              f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
            }
        }
      while (f_hz < hz_high && d < n_spectrum_bins)
        {
          if (d < band_from_d.size())
            {
              band_from_d[d] = band;
              band_from_d[d + 1] = band;
            }
          d += 2;
          f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
        }
    }
  /* count bins per band */
  for (size_t d = 0; d < n_spectrum_bins; d += 2)
    {
      int b = band_from_d[d];
      if (b != -1)
        {
          assert (b >= 0 && b < int (n_bands));
          if (band_count[b] == 0)
            band_start[b] = d;
          band_count[b]++;
        }
    }
}

size_t
NoiseBandPartition::n_bands()
{
  return band_count.size();
}

size_t
NoiseBandPartition::n_spectrum_bins()
{
  return spectrum_size;
}

void
NoiseBandPartition::noise_envelope_to_spectrum (Random& random_gen, const vector<uint16_t>& envelope, float *spectrum, double scale)
{
  assert (envelope.size() == n_bands());

  guint32 random_data[(spectrum_size + 7) / 8];

  random_gen.random_block ((spectrum_size + 7) / 8, random_data);

  zero_float_block (spectrum_size, spectrum);

  const guint8 *random_data_byte = reinterpret_cast<guint8 *> (&random_data[0]);

  for (size_t b = 0; b < n_bands(); b++)
    {
      const float value = sm_idb2factor (envelope[b]) * scale;

      size_t start = band_start[b];
      size_t end = start + band_count[b] * 2;
      for (size_t d = start; d < end; d += 2)
        {
          const guint8 r = random_data_byte[d / 2];

          spectrum[d]   = int_sinf (r) * value;
          spectrum[d+1] = int_cosf (r) * value;
        }
    }
}
