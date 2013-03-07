// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INDEX_HH
#define SPECTMORPH_INDEX_HH

#include <string>
#include <vector>

namespace SpectMorph
{

class Index
{
  std::vector<std::string> m_smsets;
  std::string              m_smset_dir;

public:
  void clear();
  bool load_file (const std::string& filename);

  const std::vector<std::string>& smsets() const;
  std::string                     smset_dir() const;
};

}

#endif
