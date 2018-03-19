// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CONFIG_HH
#define SPECTMORPH_CONFIG_HH

#include "smmicroconf.hh"
#include "smutils.hh"

namespace SpectMorph
{

class Config
{
  int m_zoom = 100;
  int m_auto_redraw = 1;

  std::string
  get_config_filename()
  {
    return sm_get_user_dir (USER_DIR_DATA) + "/config";
  }
public:
  Config()
  {
    MicroConf cfg_parser (get_config_filename());

    if (!cfg_parser.open_ok())
      return;

    while (cfg_parser.next())
      {
        int i;
        if (cfg_parser.command ("zoom", i))
          {
            m_zoom = i;
          }
        else if (cfg_parser.command ("auto_redraw", i))
          {
            m_auto_redraw = i;
          }
        else
          {
            //cfg.die_if_unknown();
          }
      }
  }

  int
  zoom()
  {
    return m_zoom;
  }

  void
  set_zoom (int z)
  {
    m_zoom = z;
  }

  bool
  auto_redraw()
  {
    return m_auto_redraw > 0;
  }

  void
  set_auto_redraw (bool b)
  {
    m_auto_redraw = b ? 1 : 0;
  }
  void
  store()
  {
    FILE *file = fopen (get_config_filename().c_str(), "w");

    if (!file)
      return;

    fprintf (file, "# this file is automatically updated by SpectMorph\n");
    fprintf (file, "# it can be manually edited, however, if you do that, be careful\n");
    fprintf (file, "zoom %d\n", m_zoom);
    fprintf (file, "auto_redraw %d\n", m_auto_redraw);

    fclose (file);
  }
};

}

#endif
