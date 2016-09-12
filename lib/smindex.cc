// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smindex.hh"
#include "smmicroconf.hh"
#include "smutils.hh"

#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

Index::Index()
{
  clear();
}

void
Index::clear()
{
  m_smset_dir = "";
  m_smsets.clear();

  m_filename = "";
  m_dir = "";
  m_load_ok = false;
}

bool
Index::load_file (const string& filename)
{
  clear();

  string expanded_filename = filename;

  size_t pos = filename.find (":"); // from instruments dir?
  if (pos != string::npos)
    {
      string category = filename.substr (0, pos);

      // special case filename starts with instruments: - typically instruments:standard
      if (category == "instruments")
        {
          string name = filename.substr (pos + 1);

          expanded_filename = sm_get_user_dir (USER_DIR_INSTRUMENTS) + "/" + name + "/index.smindex";
          m_dir = name;
        }
    }
  if (m_dir == "")
    m_filename = filename;

  MicroConf cfg (expanded_filename);
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
              char *index_dirname = g_path_get_dirname (expanded_filename.c_str());
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
  m_load_ok = true;
  return true;
}

IndexType
Index::type() const
{
  if (m_dir != "")
    return INDEX_INSTRUMENTS_DIR;
  if (m_filename != "")
    return INDEX_FILENAME;

  return INDEX_NOT_DEFINED;
}

bool
Index::load_ok() const
{
  return m_load_ok;
}

string
Index::dir() const
{
  return m_dir;
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
