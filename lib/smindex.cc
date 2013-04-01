// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smindex.hh"
#include "smmicroconf.hh"

#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

void
Index::clear()
{
  m_smset_dir = "";
  m_smsets.clear();
}

bool
Index::load_file (const string& filename)
{
  clear();

  MicroConf cfg (filename);
  if (!cfg.open_ok())
    {
      return false;
    }

  while (cfg.next())
    {
      string str;

      if (cfg.command ("smset", str))
        {
          m_smsets.push_back (str);
        }
      else if (cfg.command ("smset_dir", str))
        {
          if (!g_path_is_absolute (str.c_str()))
            {
              char *index_dirname = g_path_get_dirname (filename.c_str());
              char *absolute_path = g_build_filename (index_dirname, str.c_str(), NULL);
              m_smset_dir = absolute_path;
              g_free (absolute_path);
              g_free (index_dirname);
            }
          else
            {
              m_smset_dir = str;
            }
        }
      else
        {
          cfg.die_if_unknown();
        }
    }
  return true;
}

const vector<string>&
Index::smsets() const
{
  return m_smsets;
}

string
Index::smset_dir() const
{
  return m_smset_dir;
}
