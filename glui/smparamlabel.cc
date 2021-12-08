// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smparamlabel.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::ParamLabelModel");

ParamLabelModel::ParamLabelModel()
{
  leak_debugger.add (this);
}

ParamLabelModel::~ParamLabelModel()
{
  leak_debugger.del (this);
}
