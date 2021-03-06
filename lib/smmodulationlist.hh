// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MODULATION_LIST_HH
#define SPECTMORPH_MODULATION_LIST_HH

#include "smmath.hh"
#include "smutils.hh"
#include "smmorphoperator.hh"

#include <functional>
#include <string>

namespace SpectMorph
{

class ModulationData
{
public:
  Property::Scale property_scale = Property::Scale::NONE;

  float min_value = 0;
  float max_value = 0;
  float value_scale = 0;

  MorphOperator::ControlType    main_control_type = MorphOperator::CONTROL_GUI;
  MorphOperatorPtr              main_control_op;

  struct Entry
  {
    MorphOperator::ControlType  control_type = MorphOperator::CONTROL_SIGNAL_1;
    MorphOperatorPtr            control_op;

    bool                        bipolar = false;
    double                      amount = 0;
  };
  std::vector<Entry> entries;
};

class ModulationList
{
  ModulationData& data;
  Property&       property;

  bool                        compat = false;
  std::string                 compat_type_name;
  std::string                 compat_op_name;

  std::string                 compat_main_control_op;
  MorphOperator::ControlType  compat_main_control_type;

  bool                        have_compat_main_control_op = false;
  bool                        have_compat_main_control_type = false;
public:
  ModulationList (ModulationData& data, Property& property) :
    data (data),
    property (property)
  {
  }

  MorphOperator::ControlType
  main_control_type() const
  {
    return data.main_control_type;
  }
  MorphOperator *
  main_control_op() const
  {
    return data.main_control_op.get();
  }
  void
  set_main_control_type_and_op (MorphOperator::ControlType type, MorphOperator *op)
  {
    data.main_control_type = type;
    data.main_control_op.set (op);

    signal_main_control_changed();
    signal_modulation_changed();
  }

  size_t
  count() const
  {
    return data.entries.size();
  }
  void
  add_entry()
  {
    data.entries.emplace_back();

    signal_size_changed();
    signal_modulation_changed();
  }
  void
  update_entry (size_t index, ModulationData::Entry& new_entry)
  {
    data.entries[index] = new_entry;
    signal_modulation_changed();
  }
  const ModulationData::Entry&
  operator[] (size_t index) const
  {
    return data.entries[index];
  }
  void
  remove_entry (size_t index)
  {
    g_return_if_fail (index >= 0 && index < data.entries.size());
    data.entries.erase (data.entries.begin() + index);

    signal_size_changed();
    signal_modulation_changed();
  }
  void
  set_compat_type_and_op (const std::string& type, const std::string& op)
  {
    compat = true;

    compat_type_name = type;
    compat_op_name   = op;
  }
  void
  write_operator (OutFile& file, const std::string& name, const MorphOperatorPtr& op) /* FIXME: FILTER: maybe merge into OutFile */
  {
    std::string op_name;

    if (op.get()) // (op == NULL) => (op_name == "")
      op_name = op.get()->name();

    file.write_string (name, op_name);
  }
  void
  save (OutFile& out_file)
  {
    out_file.write_int (property.identifier() + ".modulation_main_control_type", data.main_control_type);
    write_operator (out_file, property.identifier() + ".modulation_main_control_op", data.main_control_op);
    out_file.write_int (property.identifier() + ".modulation_count", count());
    for (uint i = 0; i < data.entries.size(); i++)
      {
        out_file.write_int (property.identifier() + string_printf (".modulation_control_type_%d", i), data.entries[i].control_type);
        write_operator (out_file, property.identifier() + string_printf (".modulation_control_op_%d", i), data.entries[i].control_op);
        out_file.write_bool (property.identifier() + string_printf (".modulation_bipolar_%d", i), data.entries[i].bipolar);
        out_file.write_float (property.identifier() + string_printf (".modulation_amount_%d", i), data.entries[i].amount);
      }
  }
  bool
  load (InFile& in_file)
  {
    if (in_file.event() == InFile::STRING)
      {
        if (compat && in_file.event_name() == compat_op_name)
          {
            compat_main_control_op = in_file.event_data();
            have_compat_main_control_op = true;

            return true;
          }
      }
    else if (in_file.event() == InFile::INT)
      {
        if (compat && in_file.event_name() == compat_type_name)
          {
            compat_main_control_type = static_cast<MorphOperator::ControlType> (in_file.event_int());
            have_compat_main_control_type = true;

            return true;
          }
      }
    return false;
  }
  void
  post_load (MorphOperator::OpNameMap& op_name_map)
  {
    if (have_compat_main_control_type && have_compat_main_control_type)
      {
        data.main_control_type = compat_main_control_type;
        data.main_control_op.set (op_name_map [compat_main_control_op]);
      }
  }

  Signal<> signal_modulation_changed;
  Signal<> signal_size_changed;
  Signal<> signal_main_control_changed;
};

}

#endif
