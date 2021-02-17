// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

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
  add_property (&m_config.position, P_POSITION, "Position", "%.1f %%", 50, 0, 100);

  connect (morph_plan->signal_operator_removed, this, &MorphWavSource::on_operator_removed);
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
MorphWavSource::set_position_control_type (ControlType new_control_type)
{
  m_config.position_control_type = new_control_type;

  m_morph_plan->emit_plan_changed();
}

MorphWavSource::ControlType
MorphWavSource::position_control_type() const
{
  return m_config.position_control_type;
}

MorphOperator *
MorphWavSource::position_op() const
{
  return m_config.position_op.get();
}

void
MorphWavSource::set_position_op (MorphOperator *op)
{
  m_config.position_op.set (op);

  m_morph_plan->emit_plan_changed();
}

void
MorphWavSource::set_position_control_type_and_op (ControlType control_type, MorphOperator *op)
{
  m_config.position_control_type = control_type;
  m_config.position_op.set (op);

  m_morph_plan->emit_plan_changed();
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
  out_file.write_int ("position_control_type", m_config.position_control_type);
  write_operator (out_file, "position_op", m_config.position_op);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  load_position_op = "";

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
          else if (ifile.event_name() == "position_control_type")
            {
              m_config.position_control_type = static_cast<ControlType> (ifile.event_int());
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
          else if (ifile.event_name() == "position_op")
            {
              load_position_op = ifile.event_data();
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

void
MorphWavSource::post_load (OpNameMap& op_name_map)
{
  m_config.position_op.set (op_name_map[load_position_op]);
}

void
MorphWavSource::on_operator_removed (MorphOperator *op)
{
  // plan changed will be emitted automatically after remove, so we don't emit it here

  if (op == m_config.position_op.get())
    {
      m_config.position_op.set (nullptr);

      if (m_config.position_control_type == CONTROL_OP)
        m_config.position_control_type = CONTROL_GUI;
    }
}

MorphOperator::OutputType
MorphWavSource::output_type()
{
  return OUTPUT_AUDIO;
}

vector<MorphOperator *>
MorphWavSource::dependencies()
{
  return { (m_config.position_control_type == CONTROL_OP && m_config.play_mode == PLAY_MODE_CUSTOM_POSITION) ? m_config.position_op.get() : nullptr };
}

MorphOperatorConfig *
MorphWavSource::clone_config()
{
  Config *cfg = new Config (m_config);

  cfg->project = morph_plan()->project();

  return cfg;
}
