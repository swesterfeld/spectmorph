// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CONFIG_HH
#define SPECTMORPH_CONFIG_HH

#include "smmicroconf.hh"

namespace SpectMorph
{

class Config
{
  int m_zoom = 100;

  std::string
  get_config_filename()
  {
    std::string filename = g_get_home_dir();

    filename += "/.spectmorph/config";

    return filename;
  }
public:
  Config()
  {
    MicroConf cfg_parser (get_config_filename());

    if (!cfg_parser.open_ok())
      return;

    while (cfg_parser.next())
      {
        int z;
        if (cfg_parser.command ("zoom", z))
          {
            m_zoom = z;
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

  void
  store()
  {
    FILE *file = fopen (get_config_filename().c_str(), "w");

    if (!file)
      return;

    fprintf (file, "# this file is automatically updated by SpectMorph\n");
    fprintf (file, "# it can be manually edited, however, if you do that, be careful\n");
    fprintf (file, "zoom %d\n", m_zoom);

    fclose (file);
  }
};

}

#endif
