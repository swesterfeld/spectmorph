// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSource");

MorphWavSource::MorphWavSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);
}

MorphWavSource::~MorphWavSource()
{
  leak_debugger.del (this);
}

void
MorphWavSource::update_instrument_and_id (const string& instrument)
{
  static int64 static_id = 1;

  m_instrument    = instrument;
  m_instrument_id = static_id++;
}

void
MorphWavSource::set_instrument (const string& instrument)
{
  update_instrument_and_id (instrument);

  m_morph_plan->emit_plan_changed();
}

string
MorphWavSource::instrument()
{
  return m_instrument;
}

int64
MorphWavSource::instrument_id()
{
  return m_instrument_id;
}

const char *
MorphWavSource::type()
{
  return "SpectMorph::MorphWavSource";
}

int
MorphWavSource::insert_order()
{
  return 0;
}

bool
MorphWavSource::save (OutFile& out_file)
{
  out_file.write_string ("instrument", m_instrument);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "instrument")
            {
              update_instrument_and_id (ifile.event_data());
            }
          else
            {
              g_printerr ("bad string\n");
              return false;
            }
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
MorphWavSource::output_type()
{
  return OUTPUT_AUDIO;
}
