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

  float value = 0;              // ui slider / base value
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
  ModulationData&             data;
  Property&                   property;

  bool                        compat = false;
  std::string                 compat_type_name;
  std::string                 compat_op_name;

  std::string                 compat_main_control_op;
  MorphOperator::ControlType  compat_main_control_type;

  bool                        have_compat_main_control_op = false;
  bool                        have_compat_main_control_type = false;

  std::vector<std::string>    load_control_ops;
  std::string                 m_main_control_op;
public:
  ModulationList (ModulationData& data, Property& property);

  MorphOperator::ControlType main_control_type() const;
  MorphOperator *main_control_op() const;

  void set_main_control_type_and_op (MorphOperator::ControlType type, MorphOperator *op);

  size_t count() const;
  void add_entry();
  void update_entry (size_t index, ModulationData::Entry& new_entry);
  const ModulationData::Entry& operator[] (size_t index) const;
  void remove_entry (size_t index);
  void set_compat_type_and_op (const std::string& type, const std::string& op);
  void save (OutFile& out_file);
  std::string event_name (const std::string& id, int index = -1);
  bool split_event_name (const std::string& name, const std::string& start, int& index);
  bool load (InFile& in_file);
  void post_load (MorphOperator::OpNameMap& op_name_map);
  void get_dependencies (std::vector<MorphOperator *>& deps);

  Signal<> signal_modulation_changed;
  Signal<> signal_size_changed;
  Signal<> signal_main_control_changed;
};

}

#endif
