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
#include "smmorphlinear.hh"
#include "smhexstring.hh"
#include "smleakdebugger.hh"

#include <map>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;

static LeakDebugger leak_debugger ("SpectMorph::MorphPlan");

MorphPlan::MorphPlan()
{
  in_restore = false;

  m_structure_version = 0;

  leak_debugger.add (this);
}

void
MorphPlan::clear()
{
  m_structure_version = 0;

  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    delete (*oi);
  m_operators.clear();
  m_index.clear();
}

MorphPlan::~MorphPlan()
{
  assert (!in_restore);

  clear();

  leak_debugger.del (this);
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
  emit_index_changed();
  return result;
}

void
MorphPlan::add_operator (MorphOperator *op, const string& load_name, const string& load_id)
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
          name = Birnet::string_printf ("%s #%d", op->type_name().c_str(), i);

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
  if (load_id == "")
    {
      op->set_id (generate_id());
    }
  else
    {
      op->set_id (load_id);
    }

  m_operators.push_back (op);
  m_structure_version++;

  emit_plan_changed();
}

void
MorphPlan::set_plan_str (const string& str)
{
  vector<unsigned char> data;
  if (!HexString::decode (str, data))
    return;

  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  load (in);
  delete in;
}

BseErrorType
MorphPlan::load (GenericIn *in)
{
  in_restore = true;

  clear();
  InFile ifile (in);

  index_filename = "";

  map<string, vector<unsigned char> > blob_data_map;
  string section;
  MorphOperator *load_op = NULL;
  string         load_name;
  string         load_id;
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

                  load_op = MorphOperator::create (operator_type, this);
                  if (!load_op)
                    {
                      g_printerr ("unknown operator type %s\n", operator_type.c_str());
                    }
                }
              else if (ifile.event_name() == "name")
                {
                  load_name = ifile.event_data();
                }
              else if (ifile.event_name() == "id")
                {
                  load_id = ifile.event_data();
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

                  delete blob_in; // close blob file handle

                  GenericIn *in = MMapIn::open_mem (&blob_data[0], &blob_data[blob_data.size()]);
                  InFile blob_infile (in);
                  load_op->load (blob_infile);

                  delete in; // close memory file handle

                  add_operator (load_op, load_name, load_id);
                }
            }
        }
      else if (ifile.event() == InFile::BLOB_REF)
        {
          if (section == "operator")
            {
              if (ifile.event_name() == "data")
                {
                  assert (load_op != NULL);

                  vector<unsigned char>& blob_data = blob_data_map[ifile.event_blob_sum()];

                  GenericIn *in = MMapIn::open_mem (&blob_data[0], &blob_data[blob_data.size()]);
                  InFile blob_infile (in);
                  load_op->load (blob_infile);

                  delete in; // close memory file handle

                  add_operator (load_op, load_name, load_id);
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

  load_index (index_filename);

  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;
      op->post_load();
    }

  in_restore = false;
  emit_plan_changed();

  return BSE_ERROR_NONE;
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
  emit_plan_changed();
}

void
MorphPlan::move (MorphOperator *op, MorphOperator *op_next)
{
  vector<MorphOperator *> new_operators;

  // change op position so that op is before op_next
  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      if (*oi == op_next)
        new_operators.push_back (op);
      if (*oi != op)
        new_operators.push_back (*oi);
    }
  // handle move to the end of the operator list
  if (!op_next)
    new_operators.push_back (op);

  m_structure_version++;
  m_operators = new_operators;

  emit_plan_changed();
}

BseErrorType
MorphPlan::save (GenericOut *file)
{
  OutFile of (file, "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
  of.write_string ("index", index_filename);
  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;

      of.begin_section ("operator");
      of.write_string ("type", op->type());
      of.write_string ("name", op->name());
      of.write_string ("id", op->id());

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
  return BSE_ERROR_NONE;
}

void
MorphPlan::emit_plan_changed()
{
  if (!in_restore)
    signal_plan_changed();
}

void
MorphPlan::emit_index_changed()
{
  if (!in_restore)
    signal_index_changed();
}

string
MorphPlan::generate_id()
{
  string chars = id_chars();

  string id;
  for (size_t i = 0; i < 20; i++)
    id += chars[g_random_int_range (0, chars.size())];

  return id;
}

string
MorphPlan::id_chars()
{
  return G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS "_-.:,;/&%$*+@?!#|{}()[]<>=^";
}
