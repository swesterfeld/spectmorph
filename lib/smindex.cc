/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smindex.hh"
#include "smmicroconf.hh"

#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

bool
Index::load_file (const string& filename)
{
  MicroConf cfg (filename);

  while (cfg.next())
    {
      string str;

      if (cfg.command ("smset", str))
        {
          m_smsets.push_back (str);
        }
      else if (cfg.command ("smset_dir", str))
        {
          m_smset_dir = str;
        }
      else
        {
          cfg.die_if_unknown();
        }
    }
  return true;
}

vector<string>
Index::smsets()
{
  return m_smsets;
}

string
Index::smset_dir()
{
  return m_smset_dir;
}
