// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphenvelopemodule.hh"
#include "smmorphenvelope.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"

using namespace SpectMorph;

using std::max;

MorphEnvelopeModule::MorphEnvelopeModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
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

  double time_unit = 0;
  double beat_unit = 0;
  switch (cfg->unit)
    {
      case MorphEnvelope::UNIT_SECONDS:
        time_unit = 1;
        break;
      case MorphEnvelope::UNIT_MINUTES:
        time_unit = 60;
        break;
      case MorphEnvelope::UNIT_NOTE_1_1:
      case MorphEnvelope::UNIT_NOTE_1_2:
      case MorphEnvelope::UNIT_NOTE_1_4:
      case MorphEnvelope::UNIT_NOTE_1_8:
      case MorphEnvelope::UNIT_NOTE_1_16:
      case MorphEnvelope::UNIT_NOTE_1_32:
        beat_unit = pow (2, (MorphEnvelope::UNIT_NOTE_1_4 - cfg->unit));
        break;
      case MorphEnvelope::UNIT_NOTE_1_1T:
      case MorphEnvelope::UNIT_NOTE_1_2T:
      case MorphEnvelope::UNIT_NOTE_1_4T:
      case MorphEnvelope::UNIT_NOTE_1_8T:
      case MorphEnvelope::UNIT_NOTE_1_16T:
      case MorphEnvelope::UNIT_NOTE_1_32T:
        beat_unit = pow (2, (MorphEnvelope::UNIT_NOTE_1_4T - cfg->unit)) * 2 / 3;
        break;
      case MorphEnvelope::UNIT_NOTE_1_1D:
      case MorphEnvelope::UNIT_NOTE_1_2D:
      case MorphEnvelope::UNIT_NOTE_1_4D:
      case MorphEnvelope::UNIT_NOTE_1_8D:
      case MorphEnvelope::UNIT_NOTE_1_16D:
      case MorphEnvelope::UNIT_NOTE_1_32D:
        beat_unit = pow (2, (MorphEnvelope::UNIT_NOTE_1_4D - cfg->unit)) * 3 / 2;
        break;
    }
  if (time_unit > 0)
    {
      if (time.time_ms > last_time_ms)
        phase += (time.time_ms - last_time_ms) / (1000 * cfg->time * time_unit) * direction;
    }
  else
    {
      if (time.ppq_pos > last_ppq_pos)
        {
          /* For beat relative timing, we want to know how long the note has been playing.
           * Since there can be backwards jumps (caused by loop), we look at the delta
           * values only.  There is a small error here, during jumps, but the
           * result should be acceptable.
           */
          phase += (time.ppq_pos - last_ppq_pos) / (cfg->time * beat_unit) * direction;
        }
    }
  last_time_ms = time.time_ms;
  last_ppq_pos = time.ppq_pos;

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
          if ((phase > loop_end && direction == 1) || (phase < loop_start && direction == -1))
            {
              double pos = mod_pos (phase - loop_start, loop_len * 2);
              if (pos > loop_len)
                {
                  /* we need to change the direction of the playback because the modulo loop
                   * position is now in the "reverse" part of the loop
                   */
                  phase = loop_end - (pos - loop_len);
                  direction *= -1;
                }
              else
                {
                  /* the modulo loop position is in the same direction, just keep playing */
                  phase = loop_start + pos;
                }
            }
        }
    }
  // can happen if loop mode is changed by user while note is playing
  if (cfg->curve.loop != Curve::Loop::PING_PONG && direction == -1)
    direction = 1;

  float v;
  if (note_off_segment && phase <= note_off_p2.x)
    v = Curve::evaluate2 (phase, note_off_p1, note_off_p2);
  else
    v = cfg->curve (phase);
  v = std::clamp (v * 2 - 1, -1.f, 1.f);

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
  last_ppq_pos = time_info.ppq_pos;
  seen_note_off = false;
  note_off_segment = false;
}

void
MorphEnvelopeModule::note_off()
{
  seen_note_off = true;
  direction = 1;
  if (cfg->curve.loop != Curve::Loop::NONE)
    {
      double y = cfg->curve (phase);
      phase = cfg->curve.points[cfg->curve.loop_end].x;

      if (cfg->curve.loop_end + 1 < int (cfg->curve.points.size()))
        {
          /* for note off with a looping envelope we jump into the first segment
           * after the loop end (if available) but start at the current y position */
          note_off_p1 = cfg->curve.points[cfg->curve.loop_end];
          note_off_p2 = cfg->curve.points[cfg->curve.loop_end + 1];
          note_off_p1.y = y;
          note_off_segment = true;
        }
    }
}
