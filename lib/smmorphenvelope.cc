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
  m_config.curve.save ("curve", out_file);
  return true;
}

bool
MorphEnvelope::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile) || m_config.curve.load ("curve", ifile))
        {
          // property has been read, so we ignore the event
        }
      else
        {
          g_printerr ("bad event\n");
          return false;
        }
      ifile.next_event();
    }
  return true;
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
