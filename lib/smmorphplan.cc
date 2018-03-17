// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
#include "smutils.hh"

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

  leak_debugger.add (this);
}

/**
 * Clears MorphPlan, deletes all operators and resets all variables to return
 * to a state that is completely empty (like after creation).
 */
void
MorphPlan::clear()
{
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

/**
 * Sets and loads instrument index to be used.
 * \returns true if the instrument index was loaded successfully, false otherwise
 */
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
MorphPlan::add_operator (MorphOperator *op, AddPos add_pos, const string& load_name, const string& load_id,
                         bool load_folded)
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
          name = string_printf ("%s #%d", op->type_name().c_str(), i);

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
  op->set_folded (load_folded);

  if (add_pos == ADD_POS_AUTO)
    {
      size_t pos = 0;

      for (size_t i = 0; i < m_operators.size(); i++)
        {
          if (m_operators[i]->insert_order() <= op->insert_order())
            pos = i + 1;
        }
      m_operators.insert (m_operators.begin() + pos, op);
    }
  else
    {
      m_operators.push_back (op);
    }

  signal_need_view_rebuild();
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

/**
 * Loads MorphPlan from input stream.
 *
 * \returns BSE_ERROR_NONE if everything worked, an error otherwise
 */
Error
MorphPlan::load (GenericIn *in, ExtraParameters *params)
{
  in_restore = true;

  signal_need_view_rebuild();

  clear();
  InFile ifile (in);

  index_filename = "";

  map<string, vector<unsigned char> > blob_data_map;
  string section;
  MorphOperator *load_op = NULL;
  string         load_name;
  string         load_id;
  bool           load_folded = false;
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
      else if (params && section == params->section())
        {
          // if ExtraParameters is defined, forward all events that belong to the appropriate section
          params->handle_event (ifile);
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
      else if (ifile.event() == InFile::BOOL)
        {
          if (section == "operator")
            {
              if (ifile.event_name() == "folded")
                {
                  load_folded = ifile.event_bool();
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

                  add_operator (load_op, ADD_POS_END, load_name, load_id, load_folded);
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

                  add_operator (load_op, ADD_POS_END, load_name, load_id, load_folded);
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

  map<string, MorphOperator *> op_name2ptr;
  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;
      op_name2ptr[op->name()] = *oi;
    }
  for (vector<MorphOperator *>::iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;
      op->post_load (op_name2ptr);
    }

  in_restore = false;
  emit_plan_changed();
  emit_index_changed();

  return Error::NONE;
}

/**
 * Get MorphPlan operators.
 *
 * \returns a read-only reference to the vector containing the operators.
 */
const vector<MorphOperator*>&
MorphPlan::operators()
{
  return m_operators;
}

const Index *
MorphPlan::index()
{
  return &m_index;
}

void
MorphPlan::remove (MorphOperator *op)
{
  signal_need_view_rebuild();
  signal_operator_removed (op);

  // accessing operator contents after remove was called is an error
  delete op;

  vector<MorphOperator *>::iterator oi = m_operators.begin();
  while (oi != m_operators.end())
    {
      if (*oi == op)
        oi = m_operators.erase (oi);
      else
        oi++;
    }

  emit_plan_changed();
}

void
MorphPlan::move (MorphOperator *op, MorphOperator *op_next)
{
  signal_need_view_rebuild();

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

  m_operators = new_operators;

  emit_plan_changed();
}

Error
MorphPlan::save (GenericOut *file, ExtraParameters *params) const
{
  OutFile of (file, "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
  of.write_string ("index", index_filename);
  for (vector<MorphOperator *>::const_iterator oi = m_operators.begin(); oi != m_operators.end(); oi++)
    {
      MorphOperator *op = *oi;

      of.begin_section ("operator");
      of.write_string ("type", op->type());
      of.write_string ("name", op->name());
      of.write_string ("id", op->id());
      of.write_bool ("folded", op->folded());

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
  if (params)
    {
      of.begin_section (params->section());
      params->save (of);
      of.end_section();
    }
  return Error::NONE;
}

void
MorphPlan::emit_plan_changed()
{
  if (!in_restore)
    {
      signal_plan_changed();
    }
}

void
MorphPlan::emit_index_changed()
{
  if (!in_restore)
    {
      signal_index_changed();
    }
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

MorphPlan *
MorphPlan::clone() const
{
  // create a deep copy (by saving/loading)
  vector<unsigned char> plan_data;
  MemOut                plan_mo (&plan_data);

  save (&plan_mo);

  MorphPlan *plan_clone = new MorphPlan();
  GenericIn *in = MMapIn::open_mem (&plan_data[0], &plan_data[plan_data.size()]);
  plan_clone->load (in);
  delete in;

  return plan_clone;
}
