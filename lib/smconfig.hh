// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  std::string              m_font;
  std::string              m_font_bold;

  std::string get_config_filename();
public:
  Config();

  int   zoom() const;
  void  set_zoom (int z);

  std::vector<std::string> debug();

  std::string font() const;
  std::string font_bold() const;

  void store();
};

}

#endif
