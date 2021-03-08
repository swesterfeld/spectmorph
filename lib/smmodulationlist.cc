// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmodulationlist.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

ModulationList::ModulationList (ModulationData& data, Property& property) :
    data (data),
    property (property)
{
}

MorphOperator::ControlType
ModulationList::main_control_type() const
{
  return data.main_control_type;
}

MorphOperator *
ModulationList::main_control_op() const
{
  return data.main_control_op.get();
}

void
ModulationList::set_main_control_type_and_op (MorphOperator::ControlType type, MorphOperator *op)
{
  data.main_control_type = type;
  data.main_control_op.set (op);

  signal_main_control_changed();
  signal_modulation_changed();
}

size_t
ModulationList::count() const
{
  return data.entries.size();
}

void
ModulationList::add_entry()
{
  data.entries.emplace_back();

  signal_size_changed();
  signal_modulation_changed();
}

void
ModulationList::update_entry (size_t index, ModulationData::Entry& new_entry)
{
  data.entries[index] = new_entry;
  signal_modulation_changed();
}

const ModulationData::Entry&
ModulationList::operator[] (size_t index) const
{
  return data.entries[index];
}

void
ModulationList::remove_entry (size_t index)
{
  g_return_if_fail (index >= 0 && index < data.entries.size());
  data.entries.erase (data.entries.begin() + index);

  signal_size_changed();
  signal_modulation_changed();
}

void
ModulationList::set_compat_type_and_op (const string& type, const string& op)
{
  compat = true;

  compat_type_name = type;
  compat_op_name   = op;
}

void
ModulationList::save (OutFile& out_file)
{
  out_file.write_int (event_name ("main_control_type"), data.main_control_type);
  out_file.write_operator (event_name ("main_control_op"), data.main_control_op);
  out_file.write_int (event_name ("count"), count());
  for (uint i = 0; i < data.entries.size(); i++)
    {
      out_file.write_int (event_name ("control_type", i), data.entries[i].control_type);
      out_file.write_operator (event_name ("control_op", i), data.entries[i].control_op);
      out_file.write_bool (event_name ("bipolar", i), data.entries[i].bipolar);
      out_file.write_float (event_name ("amount", i), data.entries[i].amount);
    }
}

string
ModulationList::event_name (const string& id, int index)
{
  string s = property.identifier() + ".modulation." + id;

  if (index >= 0)
    s += string_printf ("_%d", index);

  return s;
}

static bool
starts_with (const string& key, const string& start)
{
  return key.substr (0, start.size()) == start;
}

bool
ModulationList::split_event_name (const string& name, const string& start, int& index)
{
  string prefix = event_name (start) + "_";

  if (!starts_with (name, prefix))
    return false;

  index = atoi (name.substr (prefix.length()).c_str());
  return true;
}

bool
ModulationList::load (InFile& in_file)
{
  int i;

  if (in_file.event() == InFile::STRING)
    {
      if (compat && in_file.event_name() == compat_op_name)
        {
          compat_main_control_op = in_file.event_data();
          have_compat_main_control_op = true;
        }
      else if (in_file.event_name() == event_name ("main_control_op"))
        {
          m_main_control_op = in_file.event_data();
        }
      else if (split_event_name (in_file.event_name(), "control_op", i))
        {
          load_control_ops[i] = in_file.event_data();
        }
      else
        {
          return false;
        }
    }
  else if (in_file.event() == InFile::INT)
    {
      if (compat && in_file.event_name() == compat_type_name)
        {
          compat_main_control_type = static_cast<MorphOperator::ControlType> (in_file.event_int());
          have_compat_main_control_type = true;
        }
      else if (in_file.event_name() == event_name ("main_control_type"))
        {
          data.main_control_type = static_cast<MorphOperator::ControlType> (in_file.event_int());
        }
      else if (in_file.event_name() == event_name ("count"))
        {
          data.entries.resize (in_file.event_int());
          load_control_ops.resize (in_file.event_int());
        }
      else if (split_event_name (in_file.event_name(), "control_type", i))
        {
          data.entries[i].control_type = static_cast<MorphOperator::ControlType> (in_file.event_int());
        }
      else
        {
          return false;
        }
    }
  else if (in_file.event() == InFile::BOOL)
    {
      if (split_event_name (in_file.event_name(), "bipolar", i))
        {
          data.entries[i].bipolar = in_file.event_bool();
        }
      else
        {
          return false;
        }
    }
  else if (in_file.event() == InFile::FLOAT)
    {
      if (split_event_name (in_file.event_name(), "amount", i))
        {
          data.entries[i].amount = in_file.event_float();
        }
      else
        {
          return false;
        }
    }
  else
    {
      return false;
    }
  return true;
}

void
ModulationList::post_load (MorphOperator::OpNameMap& op_name_map)
{
  if (have_compat_main_control_type && have_compat_main_control_type)
    {
      data.main_control_type = compat_main_control_type;
      data.main_control_op.set (op_name_map[compat_main_control_op]);
    }
  else
    {
      data.main_control_op.set (op_name_map[m_main_control_op]);
    }

  for (uint i = 0; i < data.entries.size(); i++)
    data.entries[i].control_op.set (op_name_map[load_control_ops[i]]);
}

void
ModulationList::get_dependencies (vector<MorphOperator *>& deps)
{
  if (data.main_control_type == MorphOperator::CONTROL_OP)
    deps.push_back (data.main_control_op.get());

  for (const auto& entry : data.entries)
    if (entry.control_type == MorphOperator::CONTROL_OP)
      deps.push_back (entry.control_op.get());
}
