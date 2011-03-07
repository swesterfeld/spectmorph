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
#include "smhexstring.hh"

#include <map>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;

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
MorphPlan::add_operator (MorphOperator *op, const string& load_name)
{
  if (load_name == "")
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
    }
  else
    {
      op->set_name (load_name);
    }

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
              of.write_string ("name", op->name());

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
      printf ("%s\n", HexString::encode (data).c_str());
      fflush (stdout);
    }
}

void
MorphPlan::set_plan_str (const string& str)
{
  in_restore = true;

  vector<unsigned char> data;
  if (!HexString::decode (str, data))
    return;

  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  InFile ifile (in);

  index_filename = "";

  map<string, vector<unsigned char> > blob_data_map;
  string section;
  MorphOperator *load_op = NULL;
  string         load_name;
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
              else if (ifile.event_name() == "name")
                {
                  load_name = ifile.event_data();
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
                  vector<unsigned char>& blob_data = blob_data_map[ifile.event_blob_sum()];
                  int ch;
                  while ((ch = blob_in->get_byte()) >= 0)
                    blob_data.push_back (ch);

                  GenericIn *in = MMapIn::open_mem (&blob_data[0], &blob_data[blob_data.size()]);
                  InFile blob_infile (in);
                  load_op->load (blob_infile);

                  add_operator (load_op, load_name);
                }
            }
        }
      else if (ifile.event() == InFile::BLOB_REF)
        {
          if (section == "operator")
            {
              if (ifile.event_name() == "data")
                {
                  vector<unsigned char>& blob_data = blob_data_map[ifile.event_blob_sum()];

                  GenericIn *in = MMapIn::open_mem (&blob_data[0], &blob_data[blob_data.size()]);
                  InFile blob_infile (in);
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

void
MorphPlan::remove (MorphOperator *op)
{
  vector<MorphOperator *>::iterator oi = m_operators.begin();
  while (oi != m_operators.end())
    {
      if (*oi == op)
        oi = m_operators.erase (oi);
      else
        oi++;
    }
  m_structure_version++;

  signal_operator_removed (op);
  signal_plan_changed();
}
