// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <string>
#include <vector>

namespace SpectMorph
{

class WavLoader
{
  std::vector<float> m_samples;
  double             m_mix_freq;

  WavLoader();
public:
  static WavLoader* load (const std::string& filename);

  ~WavLoader();

  const std::vector<float>& samples();
  double                    mix_freq();
};

};
