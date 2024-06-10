// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatormodule.hh"
#include "smmorphenvelope.hh"

namespace SpectMorph
{

class MorphEnvelopeModule : public MorphOperatorModule
{
  LeakDebugger leak_debugger { "SpectMorph::MorphEnvelopeModule" };

  const MorphEnvelope::Config *cfg = nullptr;
  double last_time_ms = 0;
  double last_ppq_pos = 0;
  double phase = 0;
  double direction = 1;
  bool   seen_note_off = false;
  bool   note_off_segment = false;
  Curve::Point note_off_p1, note_off_p2;
public:
  MorphEnvelopeModule (MorphPlanVoice *voice);

  void  set_config (const MorphOperatorConfig *cfg) override;
  float value() override;
  void  note_on (const TimeInfo& time_info) override;
  void  note_off() override;
};

}
