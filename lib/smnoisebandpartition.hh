// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NOISE_BAND_PARTITION_HH
#define SPECTMORPH_NOISE_BAND_PARTITION_HH

#include <vector>

#include <stdint.h>

#include "smrandom.hh"

namespace SpectMorph
{

class NoiseBandPartition
{
  std::vector<int> band_count;
  std::vector<int> band_start;
  size_t           spectrum_size;

public:
  NoiseBandPartition (size_t n_bands, size_t n_spectrum_bins, double mix_freq);
  void noise_envelope_to_spectrum (SpectMorph::Random& random_gen, const std::vector<uint16_t>& envelope, float *spectrum, double scale);

  size_t n_bands();
  size_t n_spectrum_bins();
};

}

#endif
