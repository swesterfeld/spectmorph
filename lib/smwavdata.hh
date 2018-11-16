// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVE_DATA_HH
#define SPECTMORPH_WAVE_DATA_HH

#include "smutils.hh"

#include <vector>

namespace SpectMorph
{

class WavData
{
  std::vector<float> m_samples;
  float              m_mix_freq;
  int                m_n_channels;
  int                m_bit_depth;
  std::string        m_error_blurb;

public:
  WavData();
  WavData (const std::vector<float>& samples, int n_channels, float mix_freq, int bit_depth);

  bool load (const std::string& filename);
  bool load_mono (const std::string& filename);

  void load (const std::vector<float>& samples, int n_channels, float mix_freq, int bit_depth);

  bool save (const std::string& filename);

  void clear();
  void prepend (const std::vector<float>& samples);

  float                       mix_freq() const;
  int                         n_channels() const;
  size_t                      n_values() const;
  int                         bit_depth() const;
  const std::vector<float>&   samples() const;
  const char                 *error_blurb() const;

  float operator[] (size_t pos) const;
};

}

#endif /* SPECTMORPH_WAVE_DATA_HH */
