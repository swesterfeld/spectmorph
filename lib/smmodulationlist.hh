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
  float min_value = 0;
  float max_value = 0;
  float value_scale = 0;

  struct Entry
  {
    MorphOperator::ControlType  control_type = MorphOperator::CONTROL_SIGNAL_1;
    MorphOperatorPtr            control_op;

    bool                        bipolar = false;
    double                      mod_amount = 0;
  };
  std::vector<Entry> entries;
};

class ModulationList
{
  ModulationData& data;

public:
  ModulationList (ModulationData& data) :
    data (data)
  {
  }

  size_t
  size() const
  {
    return data.entries.size();
  }
  void
  add_entry()
  {
    data.entries.emplace_back();
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
    signal_modulation_changed();
  }
  Signal<> signal_modulation_changed;
};

}

#endif
