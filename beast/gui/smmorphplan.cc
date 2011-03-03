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

#include "smmorphplan.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include "smoutfile.hh"
#include "smaudio.hh"
#include "sminfile.hh"
#include "smmorphsource.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlan::MorphPlan()
{
  in_restore = false;
}

MorphPlan::~MorphPlan()
{
  assert (!in_restore);
}

bool
MorphPlan::load_index (const string& filename)
{
  return index.load_file (filename);
}

void
MorphPlan::add_operator (MorphOperator *op)
{
  operators.push_back (op);

  if (!in_restore)
    {
      signal_plan_changed();

      vector<unsigned char> data;
      MemOut mo (&data);
      // we need an OutFile destructor run before we can use the data
        {
          OutFile of (&mo, "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
          for (vector<MorphOperator *>::iterator oi = operators.begin(); oi != operators.end(); oi++)
            {
              of.begin_section ("operator");
              of.end_section();
            }
        }
      for (size_t i = 0; i < data.size(); i++)
        {
          printf ("%02x", data[i]);
        }
      printf ("\n");
      fflush (stdout);
    }
}

static unsigned char
from_hex_nibble (char c)
{
  int uc = (unsigned char)c;

  if (uc >= '0' && uc <= '9') return uc - (unsigned char)'0';
  if (uc >= 'a' && uc <= 'f') return uc + 10 - (unsigned char)'a';
  if (uc >= 'A' && uc <= 'F') return uc + 10 - (unsigned char)'A';

  return 16;	// error
}

void
MorphPlan::set_plan_str (const string& str)
{
  in_restore = true;

  vector<unsigned char> data;
  string::const_iterator si = str.begin();
  while (si != str.end())
    {
      unsigned char h = from_hex_nibble (*si++);	// high nibble
      if (si == str.end())
        {
          g_printerr ("GUI: bad plan_str, end before expected end\n");
          in_restore = false;
          return;
        }

      unsigned char l = from_hex_nibble (*si++);	// low nibble
      if (h >= 16 || l >= 16)
        {
          g_printerr ("GUI: bad plan_str, no hex digit\n");
          in_restore = false;
          return;
        }
      data.push_back((h << 4) + l);
    }

  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  InFile ifile (in);

  string section;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          add_operator (new MorphSource());
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          assert (section != "");
          section = "";
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          g_printerr ("read error\n");
          break;
        }
      ifile.next_event();
    }
  delete in;

  in_restore = false;
  signal_plan_changed();
}

const vector<MorphOperator*>&
MorphPlan::get_operators()
{
  return operators;
}
