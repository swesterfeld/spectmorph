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
  m_modulation_list = std::make_unique<ModulationList> (*mod_data);

  connect (m_modulation_list->signal_modulation_changed, [this] () { signal_modulation_changed(); });
}

