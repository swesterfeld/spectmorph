// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphkeytrackmodule.hh"
#include "smmorphkeytrack.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"

using namespace SpectMorph;

MorphKeyTrackModule::MorphKeyTrackModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
}

void
MorphKeyTrackModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphKeyTrack::Config *> (op_cfg);
}

float
MorphKeyTrackModule::value()
{
  float note = sm_freq_to_note (morph_plan_voice->current_freq()) / 127;
  float v = std::clamp (cfg->curve (note) * 2 - 1, -1.f, 1.f);
  set_notify_value (0, v);
  set_notify_value (1, note);
  return v;
}
