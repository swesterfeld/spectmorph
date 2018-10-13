// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourcemodule.hh"
#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSourceModule");

MorphWavSourceModule::MorphWavSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);
}

MorphWavSourceModule::~MorphWavSourceModule()
{
  leak_debugger.del (this);
}

LiveDecoderSource *
MorphWavSourceModule::source()
{
  return nullptr;
}

void
MorphWavSourceModule::set_config (MorphOperator *op)
{
  MorphWavSource *source = dynamic_cast<MorphWavSource *> (op);

  printf ("MorphWavSourceModule::set_config: using instrument=%s\n", source->instrument().c_str());
}
