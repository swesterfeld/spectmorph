// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MODULATION_LIST_HH
#define SPECTMORPH_MODULATION_LIST_HH

#include "smmath.hh"
#include "smutils.hh"

#include <functional>
#include <string>

namespace SpectMorph
{

class ModulationList
{
public:
  struct Entry
  {
    MorphOperator::ControlType  control_type = MorphOperator::CONTROL_SIGNAL_1;
    MorphOperatorPtr            control_op;

    bool                        bipolar = false;
    double                      mod_amount = 0;
  };
protected:
  std::vector<Entry> entries;
public:
  size_t
  size() const
  {
    return entries.size();
  }
  void
  add_entry()
  {
    entries.emplace_back();
  }
  Entry&
  operator[] (size_t index)
  {
    return entries[index];
  }
  const Entry&
  operator[] (size_t index) const
  {
    return entries[index];
  }
  void
  remove_entry (size_t index)
  {
    g_return_if_fail (index >= 0 && index < entries.size());
    entries.erase (entries.begin() + index);
  }
};

}

#endif
