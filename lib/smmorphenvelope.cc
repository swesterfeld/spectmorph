// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphenvelope.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphEnvelope");

MorphEnvelope::MorphEnvelope (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

  m_config.curve.points.emplace_back (Curve::Point {0, 0});
  m_config.curve.points.emplace_back (Curve::Point {0.5, 1});
  m_config.curve.points.emplace_back (Curve::Point {1, 0});
  m_config.curve.loop = Curve::Loop::SUSTAIN;

  EnumInfo unit_enum_info (
    {
      { UNIT_SECONDS,    "Seconds" },
      { UNIT_MINUTES,    "Minutes" },
      { UNIT_NOTE_1_1,   "1/1" },
      { UNIT_NOTE_1_2,   "1/2" },
      { UNIT_NOTE_1_4,   "1/4" },
      { UNIT_NOTE_1_8,   "1/8" },
      { UNIT_NOTE_1_16,  "1/16" },
      { UNIT_NOTE_1_32,  "1/32" },
      { UNIT_NOTE_1_1T,  "1/1 triplet" },
      { UNIT_NOTE_1_2T,  "1/2 triplet" },
      { UNIT_NOTE_1_4T,  "1/4 triplet" },
      { UNIT_NOTE_1_8T,  "1/8 triplet" },
      { UNIT_NOTE_1_16T, "1/16 triplet" },
      { UNIT_NOTE_1_32T, "1/32 triplet" },
      { UNIT_NOTE_1_1D,  "1/1 dotted" },
      { UNIT_NOTE_1_2D,  "1/2 dotted" },
      { UNIT_NOTE_1_4D,  "1/4 dotted" },
      { UNIT_NOTE_1_8D,  "1/8 dotted" },
      { UNIT_NOTE_1_16D, "1/16 dotted" },
      { UNIT_NOTE_1_32D, "1/32 dotted" },
    });
  add_property_enum (&m_config.unit, P_UNIT, "Unit", UNIT_SECONDS, unit_enum_info);
  add_property_log (&m_config.time, P_TIME, "Time", "%.3f", 1, 1 / 50., 50);
}

MorphEnvelope::~MorphEnvelope()
{
  leak_debugger.del (this);
}

const char *
MorphEnvelope::type()
{
  return "SpectMorph::MorphEnvelope";
}

int
MorphEnvelope::insert_order()
{
  return 200;
}

bool
MorphEnvelope::save (OutFile& out_file)
{
  write_properties (out_file);
  m_config.curve.save ("curve", out_file);
  return true;
}

bool
MorphEnvelope::load (InFile& ifile)
{
  bool read_ok = true;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile) || m_config.curve.load ("curve", ifile))
        {
          // property has been read, so we ignore the event
        }
      else
        {
          report_bad_event (read_ok, ifile);
        }
      ifile.next_event();
    }
  return read_ok;
}

MorphOperator::OutputType
MorphEnvelope::output_type()
{
  return OUTPUT_CONTROL;
}

MorphOperatorConfig *
MorphEnvelope::clone_config()
{
  return new Config (m_config);
}

const Curve&
MorphEnvelope::curve() const
{
  return m_config.curve;
}

void
MorphEnvelope::set_curve (const Curve& curve)
{
  m_config.curve = curve;
  m_morph_plan->emit_plan_changed();
}
