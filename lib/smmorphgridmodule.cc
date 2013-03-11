// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridmodule.hh"
#include "smmorphgrid.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphGridModule");

MorphGridModule::MorphGridModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 0)
{
  leak_debugger.add (this);
}

MorphGridModule::~MorphGridModule()
{
  leak_debugger.del (this);
}

void
MorphGridModule::set_config (MorphOperator *op)
{
  MorphGrid *source = dynamic_cast<MorphGrid *> (op);
}
