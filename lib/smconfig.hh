// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CONFIG_HH
#define SPECTMORPH_CONFIG_HH

#include "smutils.hh"

#include <vector>

namespace SpectMorph
{

class Config
{
  int                      m_zoom = 100;
  std::vector<std::string> m_debug;

  std::string get_config_filename();
public:
  Config();

  int   zoom() const;
  void  set_zoom (int z);

  std::vector<std::string> debug();

  void store();
};

}

#endif
