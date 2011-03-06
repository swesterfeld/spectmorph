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
#include "smmorphoutput.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlan::MorphPlan()
{
  in_restore = false;

  m_structure_version = 0;

  signal_plan_changed.connect (sigc::mem_fun (*this, &MorphPlan::on_plan_changed));
}

MorphPlan::~MorphPlan()
{
  assert (!in_restore);
}

bool
MorphPlan::load_index (const string& filename)
{
  bool result = false;
  if (filename != "")
    {
      result = m_index.load_file (filename);
    }
  index_filename = filename;
  signal_index_changed();
  return result;
}

void
MorphPlan::add_operator (MorphOperator *op)
{
  // generate uniq name
  string name;
  bool   uniq;
  int    i = 0;

  do
    {
      i++;
      uniq = true;
      name = Birnet::string_printf ("%s #%d",
                string (op->type()).substr (string ("SpectMorph::Morph").size()).c_str(), i);

      for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
        {
          if ((*oi)->name() == name)
            uniq = false;
        }
    }
  while (!uniq);
  op->set_name (name);

  m_operators.push_back (op);
  m_structure_version++;
  if (!in_restore)
    {
      signal_plan_changed();
    }
}

void
MorphPlan::on_plan_changed()
{
  if (!in_restore)
    {
      vector<unsigned char> data;
      MemOut mo (&data);
      // we need an OutFile destructor run before we can use the data
        {
          OutFile of (&mo, "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
          of.write_string ("index", index_filename);
          for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
            {
              MorphOperator *op = *oi;

              of.begin_section ("operator");
              of.write_string ("type", op->type());

              vector<unsigned char> op_data;
              MemOut                op_mo (&op_data);
              // need an OutFile destructor run before op_data is ready
              {
                OutFile op_of (&op_mo, op->type(), SPECTMORPH_BINARY_FILE_VERSION);
                op->save (op_of);
              }

              of.write_blob ("data", &op_data[0], op_data.size());
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

  index_filename = "";

  string section;
  MorphOperator *load_op = NULL;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          assert (section != "");
          section = "";
        }
      else if (ifile.event() == InFile::STRING)
        {
          if (section == "")
            {
              if (ifile.event_name() == "index")
                {
                  index_filename = ifile.event_data();
                }
            }
          else if (section == "operator")
            {
              if (ifile.event_name() == "type")
                {
                  string operator_type = ifile.event_data();

                  if (operator_type == "SpectMorph::MorphSource")
                    {
                      load_op = new MorphSource (this);
                    }
                  else if (operator_type == "SpectMorph::MorphOutput")
                    {
                      load_op = new MorphOutput (this);
                    }
                  else
                    {
                      g_printerr ("unknown operator type %s", operator_type.c_str());
                      load_op = NULL;
                    }
                }
            }
        }
      else if (ifile.event() == InFile::BLOB)
        {
          if (section == "operator")
            {
              if (ifile.event_name() == "data")
                {
                  assert (load_op != NULL);

                  GenericIn *blob_in = ifile.open_blob();
                  InFile blob_infile (blob_in);
                  load_op->load (blob_infile);

                  add_operator (load_op);
                }
            }
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          g_printerr ("read error\n");
          break;
        }
      ifile.next_event();
    }
  delete in;

  load_index (index_filename);

  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;
      op->post_load();
    }

  in_restore = false;
  signal_plan_changed();
}

const vector<MorphOperator*>&
MorphPlan::operators()
{
  return m_operators;
}

Index *
MorphPlan::index()
{
  return &m_index;
}

int
MorphPlan::structure_version()
{
  return m_structure_version;
}
