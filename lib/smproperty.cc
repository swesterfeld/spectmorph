// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smproperty.hh"
#include "smmodulationlist.hh"

using namespace SpectMorph;

Property::Property()
{
}

Property::~Property()
{
}

void
Property::set_modulation_data (ModulationData *mod_data)
{
  Range r = float_range();
  mod_data->min_value = r.min_value;
  mod_data->max_value = r.max_value;

  auto property_scale = float_scale();
  mod_data->property_scale = property_scale;

  switch (property_scale)
  {
    case Scale::LOG:
      mod_data->value_scale = log2f (r.max_value / r.min_value);
      break;
    case Scale::LINEAR:
      mod_data->value_scale = r.max_value - r.min_value;
      break;
    default:
      mod_data->value_scale = 0;
      break;
  }

  m_modulation_list = std::make_unique<ModulationList> (*mod_data);

  connect (m_modulation_list->signal_modulation_changed, [this] () { signal_modulation_changed(); });
}

