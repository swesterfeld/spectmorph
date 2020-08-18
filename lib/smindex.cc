// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smindex.hh"
#include "smmicroconf.hh"
#include "smutils.hh"
#include "config.h"

#include <glib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
  m_groups.clear();

  m_expanded_filename = "";
  m_filename = "";
  m_dir = "";
  m_load_ok = false;
}

// just check version, ignore rest
static bool
version_ok (const string& filename)
{
  MicroConf cfg (filename);
  if (cfg.open_ok())
    {
      while (cfg.next())
        {
          string version;

          // could be relaxed to a minimum version check
          if (cfg.command ("version", version))
            return version == PACKAGE_VERSION;
        }
    }
  return false;
}

bool
Index::load_file (const string& filename)
{
  clear();

  m_expanded_filename = filename;

  size_t pos = filename.find (":"); // from instruments dir?
  if (pos != string::npos)
    {
      string category = filename.substr (0, pos);

      // special case filename starts with instruments: - typically instruments:standard
      if (category == "instruments")
        {
          string name = filename.substr (pos + 1);

          string user_filename = sm_get_user_dir (USER_DIR_INSTRUMENTS) + "/" + name + "/index.smindex";
          string inst_filename = sm_get_install_dir (INSTALL_DIR_INSTRUMENTS) + "/" + name + "/index.smindex";

          if (file_exists (user_filename) && version_ok (user_filename))
            {
              // if user has instruments, they will be used
              m_expanded_filename = user_filename;
            }
          else if (file_exists (inst_filename) && version_ok (inst_filename))
            {
              // if user doesn't have instruments, but system wide instruments are installed, they will be used
              m_expanded_filename = inst_filename;
            }
          else
            {
              // if no instruments were found, we show the user dir in error messages
              // as the user should have write access to that location
              m_expanded_filename = user_filename;
            }
          m_dir = name;
        }
    }
  if (m_dir == "")
    m_filename = filename;

  if (!version_ok (m_expanded_filename))
    return false;

  MicroConf cfg (m_expanded_filename);
  if (!cfg.open_ok())
    {
      return false;
    }

  Group group;
  group.group = "-no-group-";
  while (cfg.next())
    {
      string str, label;

      if (cfg.command ("smset", str))
        {
          m_smsets.push_back (str);
          group.instruments.push_back (Instrument ({ str, str }));
        }
      else if (cfg.command ("smset", str, label))
        {
          m_smsets.push_back (str);
          group.instruments.push_back (Instrument ({ str, label }));
        }
      else if (cfg.command ("group", str))
        {
          if (group.instruments.size())
            m_groups.push_back (group);

          group = Group();
          group.group = str;
        }
      else if (cfg.command ("smset_dir", str))
        {
          if (!g_path_is_absolute (str.c_str()))
            {
              char *index_dirname = g_path_get_dirname (m_expanded_filename.c_str());
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
      else if (cfg.command ("version", str))
        {
          // ignore here, we check this before starting to load
        }
      else
        {
          cfg.die_if_unknown();
        }
    }
  if (group.instruments.size())
    m_groups.push_back (group);
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

string
Index::filename() const
{
  return m_filename;
}

string
Index::expanded_filename() const
{
  return m_expanded_filename;
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

const vector<Index::Group>&
Index::groups() const
{
  return m_groups;
}

string
Index::label_to_smset (const std::string& label) const
{
  for (auto group : m_groups)
    {
      for (auto instrument : group.instruments)
        {
          if (instrument.label == label)
            return instrument.smset;
        }
    }
  return "";
}

string
Index::smset_to_label (const std::string& smset) const
{
  for (auto group : m_groups)
    {
      for (auto instrument : group.instruments)
        {
          if (instrument.smset == smset)
            return instrument.label;
        }
    }
  return "";
}
