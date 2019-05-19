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
MorphWavSource::set_object_id (int id)
{
  // object id to use in Project
  m_object_id = id;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::object_id()
{
  return m_object_id;
}

void
MorphWavSource::set_INST (int inst)
{
  m_INST = inst;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::INST()
{
  return m_INST;
}

void
MorphWavSource::set_lv2_filename (const string& filename)
{
  m_lv2_filename = filename;

  m_morph_plan->emit_plan_changed();
}

string
MorphWavSource::lv2_filename()
{
  return m_lv2_filename;
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
  out_file.write_int ("object_id", m_object_id);
  out_file.write_int ("INST", m_INST);
  out_file.write_string ("lv2_filename", m_lv2_filename);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "object_id")
            {
              m_object_id = ifile.event_int();
            }
          else if (ifile.event_name() == "INST")
            {
              m_INST = ifile.event_int();
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "lv2_filename")
            {
              m_lv2_filename = ifile.event_data();
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
