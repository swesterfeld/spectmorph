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

#include "smhexstring.hh"

#include <glib.h>
#include <birnet/birnet.hh>

using namespace SpectMorph;

using std::string;
using std::vector;

static unsigned char
from_hex_nibble (char c)
{
  int uc = (unsigned char)c;

  if (uc >= '0' && uc <= '9') return uc - (unsigned char)'0';
  if (uc >= 'a' && uc <= 'f') return uc + 10 - (unsigned char)'a';
  if (uc >= 'A' && uc <= 'F') return uc + 10 - (unsigned char)'A';

  return 16;	// error
}

bool
HexString::decode (const string& str, vector<unsigned char>& out)
{
  string::const_iterator si = str.begin();
  while (si != str.end())
    {
      unsigned char h = from_hex_nibble (*si++);	// high nibble
      if (si == str.end())
        {
          g_printerr ("HexString::decode end before expected end\n");
          return false;
        }

      unsigned char l = from_hex_nibble (*si++);	// low nibble
      if (h >= 16 || l >= 16)
        {
          g_printerr ("HexString::decode: no hex digit\n");
          return false;
        }
      out.push_back((h << 4) + l);
    }
  return true;
}

string
HexString::encode (const vector<unsigned char>& data)
{
  string out;
  for (size_t i = 0; i < data.size(); i++)
    {
      out += Birnet::string_printf ("%02x", data[i]);
    }
  return out;
}
