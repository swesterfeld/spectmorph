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
  int                      m_auto_redraw = 1;
  std::vector<std::string> m_debug;

  std::string get_config_filename();
public:
  Config();

  int   zoom();
  void  set_zoom (int z);

  bool  auto_redraw();
  void  set_auto_redraw (bool b);

  std::vector<std::string> debug();

  void store();
};

}

#endif
