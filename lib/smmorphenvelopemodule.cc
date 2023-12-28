// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphenvelopemodule.hh"
#include "smmorphenvelope.hh"
#include "smleakdebugger.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"

using namespace SpectMorph;

using std::max;

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

/* perform positive modulo, wrap pos into range [0:len] */
static double
mod_pos (double pos, double len)
{
  if (len < 1e-17) // avoid division by zero
    return 0;
  pos = fmod (pos, len);
  if (pos < 0)     // fmod for negative input produces negative output
    pos += len;
  return pos;
}

float
MorphEnvelopeModule::value()
{
  TimeInfo time = time_info();

  if (time.time_ms > last_time_ms)
    phase += (time.time_ms - last_time_ms) / 1000 * direction;
  last_time_ms = time.time_ms;

  if (!seen_note_off)
    {
      const double loop_start = cfg->curve.points[cfg->curve.loop_start].x;
      const double loop_end   = cfg->curve.points[cfg->curve.loop_end].x;
      const double loop_len   = loop_end - loop_start;

      if (cfg->curve.loop == Curve::Loop::SUSTAIN && phase > loop_start)
        {
          phase = loop_start;
        }
      else if (cfg->curve.loop == Curve::Loop::FORWARD && phase > loop_end)
        {
          phase = loop_start + mod_pos (phase - loop_start, loop_len);
        }
      else if (cfg->curve.loop == Curve::Loop::PING_PONG)
        {
          if (phase > loop_end || (phase < loop_start && direction == -1))
            {
              double pos = mod_pos (phase - loop_start, loop_len * 2);
              if (pos > loop_len)
                {
                  // backwards section of the ping-pong loop
                  phase = loop_end - (pos - loop_len);
                  direction = -1;
                }
              else
                {
                  // forwards section of the ping-pong loop
                  phase = loop_start + pos;
                  direction = 1;
                }
            }
        }
    }
  // can happen if loop mode is changed by user while note is playing
  if (cfg->curve.loop != Curve::Loop::PING_PONG && direction == -1)
    direction = 1;

  float v = std::clamp (cfg->curve (phase) * 2 - 1, -1.f, 1.f);
  set_notify_value (0, v);
  set_notify_value (1, phase);
  return v;
}

void
MorphEnvelopeModule::note_on (const TimeInfo& time_info)
{
  phase = 0;
  direction = 1;
  last_time_ms = time_info.time_ms;
  seen_note_off = false;
}

void
MorphEnvelopeModule::note_off()
{
  seen_note_off = true;
  direction = 1;
}
