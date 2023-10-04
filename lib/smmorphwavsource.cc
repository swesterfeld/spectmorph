// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include "smproject.hh"
#include "smzip.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSource");

MorphWavSource::MorphWavSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

  EnumInfo play_mode_enum_info (
    {
      { PLAY_MODE_STANDARD,        "Standard" },
      { PLAY_MODE_CUSTOM_POSITION, "Custom Position" }
    });

  add_property_enum (&m_config.play_mode, P_PLAY_MODE, "Play Mode", PLAY_MODE_STANDARD, play_mode_enum_info);
  auto position = add_property (&m_config.position_mod, P_POSITION, "Position", "%.1f %%", 50, 0, 100);
  position->modulation_list()->set_compat_type_and_op ("position_control_type", "position_op");

  UserInstrumentIndex *user_instrument_index = morph_plan->project()->user_instrument_index();
  connect (user_instrument_index->signal_instrument_updated, this, &MorphWavSource::on_instrument_updated);
}

MorphWavSource::~MorphWavSource()
{
  leak_debugger.del (this);
}

void
MorphWavSource::set_object_id (int id)
{
  // object id to use in Project
  m_config.object_id = id;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::object_id()
{
  return m_config.object_id;
}

void
MorphWavSource::set_instrument (int instrument)
{
  m_instrument = instrument;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::instrument()
{
  return m_instrument;
}

void
MorphWavSource::set_bank (const string& bank)
{
  if (m_bank != bank)
    {
      m_bank = bank;
      m_instrument = 1;

      m_morph_plan->emit_plan_changed();
    }
}

string
MorphWavSource::bank()
{
  return m_bank;
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

void
MorphWavSource::on_instrument_updated (const std::string& bank, int number, const Instrument *new_instrument)
{
  if (bank == m_bank && number == m_instrument)
    {
      auto project  = m_morph_plan->project();
      Instrument *instrument = project->get_instrument (this);

      if (new_instrument->size())
        {
          ZipWriter new_inst_writer;
          new_instrument->save (new_inst_writer);
          ZipReader new_inst_reader (new_inst_writer.data());
          instrument->load (new_inst_reader);
        }
      else
        {
          instrument->clear();
        }
      project->rebuild (this);
      project->state_changed();
    }
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
  write_properties (out_file);

  out_file.write_int ("object_id", m_config.object_id);
  out_file.write_int ("instrument", m_instrument);
  out_file.write_string ("lv2_filename", m_lv2_filename);
  out_file.write_string ("bank", m_bank);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile))
        {
          // property has been read, so we ignore the event
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "object_id")
            {
              m_config.object_id = ifile.event_int();
            }
          else if (ifile.event_name() == "instrument")
            {
              m_instrument = ifile.event_int();
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
          else if (ifile.event_name() == "bank")
            {
              m_bank = ifile.event_data();
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

vector<MorphOperator *>
MorphWavSource::dependencies()
{
  vector<MorphOperator *> deps;

  if (m_config.play_mode == PLAY_MODE_CUSTOM_POSITION)
    get_property_dependencies (deps, { P_POSITION });
  return deps;
}

MorphOperatorConfig *
MorphWavSource::clone_config()
{
  Config *cfg = new Config (m_config);

  cfg->project = morph_plan()->project();

  return cfg;
}
