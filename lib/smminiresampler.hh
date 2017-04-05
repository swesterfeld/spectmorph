// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MINI_RESAMPLER_HH
#define SPECTMORPH_MINI_RESAMPLER_HH

#include "smwavdata.hh"

#include <vector>

namespace SpectMorph
{

class MiniResampler
{
  std::vector<float> m_samples;
public:
  MiniResampler (const WavData& wav_data, double speedup_factor);

  int read (size_t pos, size_t block_size, float *out);
  uint64 length();
};

};

#endif
