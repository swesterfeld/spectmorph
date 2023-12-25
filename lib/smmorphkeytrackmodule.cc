// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphkeytrackmodule.hh"
#include "smmorphkeytrack.hh"
#include "smleakdebugger.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphKeyTrackModule");

MorphKeyTrackModule::MorphKeyTrackModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);
}

MorphKeyTrackModule::~MorphKeyTrackModule()
{
  leak_debugger.del (this);
}

void
MorphKeyTrackModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphKeyTrack::Config *> (op_cfg);
}

float
MorphKeyTrackModule::value()
{
  double v = std::clamp (sm_freq_to_note (morph_plan_voice->current_freq()) / 127., 0.0, 1.0);
  v = std::clamp (cfg->curve (v) * 2 - 1, -1.0, 1.0);
  set_notify_value (v);
  return v;
}
