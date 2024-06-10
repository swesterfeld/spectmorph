// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatormodule.hh"
#include "smmorphkeytrack.hh"

namespace SpectMorph
{

class MorphKeyTrackModule : public MorphOperatorModule
{
  LeakDebugger leak_debugger { "SpectMorph::MorphKeyTrackModule" };

  const MorphKeyTrack::Config *cfg = nullptr;
public:
  MorphKeyTrackModule (MorphPlanVoice *voice);

  void  set_config (const MorphOperatorConfig *cfg) override;
  float value() override;
};

}
