// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSource");

MorphWavSourceProperties::MorphWavSourceProperties (MorphWavSource *wav_source) :
  position (wav_source, "Position", "%.1f %%", 0, 100, &MorphWavSource::position, &MorphWavSource::set_position)
{
}

MorphWavSource::MorphWavSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

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
  m_object_id = id;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::object_id()
{
  return m_object_id;
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
MorphWavSource::set_play_mode (PlayMode play_mode)
{
  m_play_mode = play_mode;

  m_morph_plan->emit_plan_changed();
}

MorphWavSource::PlayMode
MorphWavSource::play_mode() const
{
  return m_play_mode;
}

void
MorphWavSource::set_position_control_type (ControlType new_control_type)
{
  m_position_control_type = new_control_type;

  m_morph_plan->emit_plan_changed();
}

MorphWavSource::ControlType
MorphWavSource::position_control_type() const
{
  return m_position_control_type;
}

void
MorphWavSource::set_position (float new_position)
{
  m_position = new_position;

  m_morph_plan->emit_plan_changed();
}

float
MorphWavSource::position() const
{
  return m_position;
}

MorphOperator *
MorphWavSource::position_op() const
{
  return m_position_op;
}

void
MorphWavSource::set_position_op (MorphOperator *op)
{
  m_position_op = op;

  m_morph_plan->emit_plan_changed();
}

void
MorphWavSource::set_position_control_type_and_op (ControlType control_type, MorphOperator *op)
{
  m_position_control_type = control_type;
  m_position_op           = op;

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
  out_file.write_int ("object_id", m_object_id);
  out_file.write_int ("instrument", m_instrument);
  out_file.write_string ("lv2_filename", m_lv2_filename);
  out_file.write_int ("play_mode", m_play_mode);
  out_file.write_int ("position_control_type", m_position_control_type);
  out_file.write_float ("position", m_position);
  MorphOperatorPtr xpp; /* FIXME: CONFIG */
  xpp.set (m_position_op);
  write_operator (out_file, "position_op", xpp);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  load_position_op = "";

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "object_id")
            {
              m_object_id = ifile.event_int();
            }
          else if (ifile.event_name() == "instrument")
            {
              m_instrument = ifile.event_int();
            }
          else if (ifile.event_name() == "play_mode")
            {
              m_play_mode = static_cast<PlayMode> (ifile.event_int());
            }
          else if (ifile.event_name() == "position_control_type")
            {
              m_position_control_type = static_cast<ControlType> (ifile.event_int());
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (ifile.event_name() == "position")
            {
              m_position = ifile.event_float();
            }
          else
            {
              g_printerr ("bad float\n");
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
  m_position_op = op_name_map[load_position_op];
}

void
MorphWavSource::on_operator_removed (MorphOperator *op)
{
  // plan changed will be emitted automatically after remove, so we don't emit it here

  if (op == m_position_op)
    {
      m_position_op = nullptr;

      if (m_position_control_type == CONTROL_OP)
        m_position_control_type = CONTROL_GUI;
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
  return { (m_position_control_type == CONTROL_OP && m_play_mode == PLAY_MODE_CUSTOM_POSITION) ? m_position_op : nullptr };
}
