// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphenvelopemodule.hh"
#include "smmorphenvelope.hh"
#include "smleakdebugger.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphEnvelopeModule");

MorphEnvelopeModule::MorphEnvelopeModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);
}

MorphEnvelopeModule::~MorphEnvelopeModule()
{
  leak_debugger.del (this);
}

void
MorphEnvelopeModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphEnvelope::Config *> (op_cfg);
}

float
MorphEnvelopeModule::value()
{
  TimeInfo time = time_info();

  if (time.time_ms > last_time_ms)
    phase += (time.time_ms - last_time_ms) / 1000;
  last_time_ms = time.time_ms;

  if (cfg->curve.loop == Curve::Loop::SUSTAIN && !seen_note_off && phase > cfg->curve.points[cfg->curve.loop_start].x)
    phase = cfg->curve.points[cfg->curve.loop_start].x;

  float v = std::clamp (cfg->curve (phase) * 2 - 1, -1.f, 1.f);
  set_notify_value (v);
  return v;
}

void
MorphEnvelopeModule::note_on (const TimeInfo& time_info)
{
  phase = 0;
  last_time_ms = time_info.time_ms;
  seen_note_off = false;
}

void
MorphEnvelopeModule::note_off()
{
  seen_note_off = true;
}
