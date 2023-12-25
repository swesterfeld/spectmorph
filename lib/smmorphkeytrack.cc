// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphkeytrack.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphKeyTrack");

MorphKeyTrack::MorphKeyTrack (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

  m_config.curve.points.emplace_back (Curve::Point {0, 0});
  m_config.curve.points.emplace_back (Curve::Point {1, 1});
}

MorphKeyTrack::~MorphKeyTrack()
{
  leak_debugger.del (this);
}

const char *
MorphKeyTrack::type()
{
  return "SpectMorph::MorphKeyTrack";
}

int
MorphKeyTrack::insert_order()
{
  return 200;
}

bool
MorphKeyTrack::save (OutFile& out_file)
{
  return true;
}

bool
MorphKeyTrack::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile))
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
MorphKeyTrack::output_type()
{
  return OUTPUT_CONTROL;
}

MorphOperatorConfig *
MorphKeyTrack::clone_config()
{
  return new Config (m_config);
}

const Curve&
MorphKeyTrack::curve() const
{
  return m_config.curve;
}

void
MorphKeyTrack::set_curve (const Curve& curve)
{
  m_config.curve = curve;
  m_morph_plan->emit_plan_changed();
}
