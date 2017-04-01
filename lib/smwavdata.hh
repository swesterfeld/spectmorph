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
  WavData (const std::vector<float>& samples, int n_channels, float mix_freq);

  Error load (const std::string& filename);
  void  load (const std::vector<float>& samples, int n_channels, float mix_freq);

  void clear();
  void prepend (const std::vector<float>& samples);

  float                       mix_freq() const;
  int                         n_channels() const;
  size_t                      n_values() const;
  const std::vector<float>&   samples() const;
  float operator[] (size_t pos) const;
};

}

#endif /* SPECTMORPH_WAVE_DATA_HH */
