// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatormodule.hh"
#include "smmorphenvelope.hh"

namespace SpectMorph
{

class MorphEnvelopeModule : public MorphOperatorModule
{
  const MorphEnvelope::Config *cfg = nullptr;
  double last_time_ms = 0;
  double phase = 0;
  bool   seen_note_off = false;
public:
  MorphEnvelopeModule (MorphPlanVoice *voice);
  ~MorphEnvelopeModule();

  void  set_config (const MorphOperatorConfig *cfg) override;
  float value() override;
  void  note_on (const TimeInfo& time_info) override;
  void  note_off() override;
};

}
