// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVE_DATA_HH
#define SPECTMORPH_WAVE_DATA_HH

#include "smutils.hh"

namespace SpectMorph
{

class WavData
{
  std::vector<float> m_samples;
  float              m_mix_freq;
  int                m_n_channels;

public:
  WavData();

  Error load (const std::string& filename);
  void clear();

  float                       mix_freq() const;
  int                         n_channels() const;
  const std::vector<float>&   samples() const;
};

}

#endif /* SPECTMORPH_WAVE_DATA_HH */
