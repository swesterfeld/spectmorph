// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlinear.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphLinear");

MorphLinear::MorphLinear (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  connect (morph_plan, SIGNAL (operator_removed (MorphOperator *)), this, SLOT (on_operator_removed (MorphOperator *)));

  m_left_op = NULL;
  m_right_op = NULL;
  m_control_op = NULL;
  m_morphing = 0;
  m_control_type = CONTROL_GUI;
  m_db_linear = false;
  m_use_lpc = false;

  leak_debugger.add (this);
}

MorphLinear::~MorphLinear()
{
  leak_debugger.del (this);
}

const char *
MorphLinear::type()
{
  return "SpectMorph::MorphLinear";
}

bool
MorphLinear::save (OutFile& out_file)
{
  write_operator (out_file, "left", m_left_op);
  write_operator (out_file, "right", m_right_op);
  write_operator (out_file, "control", m_control_op);
  out_file.write_float ("morphing", m_morphing);
  out_file.write_int ("control_type", m_control_type);
  out_file.write_bool ("db_linear", m_db_linear);
  out_file.write_bool ("use_lpc", m_use_lpc);

  return true;
}

bool
MorphLinear::load (InFile& ifile)
{
  load_left    = "";
  load_right   = "";
  load_control = "";

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "left")
            {
              load_left = ifile.event_data();
            }
          else if (ifile.event_name() == "right")
            {
              load_right = ifile.event_data();
            }
          else if (ifile.event_name() == "control")
            {
              load_control = ifile.event_data();
            }
          else
            {
              g_printerr ("bad string\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (ifile.event_name() == "morphing")
            {
              m_morphing = ifile.event_float();
            }
          else
            {
              g_printerr ("bad float\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "control_type")
            {
              m_control_type = static_cast<ControlType> (ifile.event_int());
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::BOOL)
        {
          if (ifile.event_name() == "db_linear")
            {
              m_db_linear = ifile.event_bool();
            }
          else if (ifile.event_name() == "use_lpc")
            {
              m_use_lpc = ifile.event_bool();
            }
          else
            {
              g_printerr ("bad bool\n");
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
MorphLinear::post_load (OpNameMap& op_name_map)
{
  m_left_op = op_name_map[load_left];
  m_right_op = op_name_map[load_right];
  m_control_op = op_name_map[load_control];
}

MorphOperator::OutputType
MorphLinear::output_type()
{
  return OUTPUT_AUDIO;
}

void
MorphLinear::on_operator_removed (MorphOperator *op)
{
}

MorphOperator *
MorphLinear::left_op()
{
  return m_left_op;
}

MorphOperator *
MorphLinear::right_op()
{
  return m_right_op;
}

MorphOperator *
MorphLinear::control_op()
{
  return m_control_op;
}

void
MorphLinear::set_left_op (MorphOperator *op)
{
  m_left_op = op;

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_right_op (MorphOperator *op)
{
  m_right_op = op;

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_control_op (MorphOperator *op)
{
  m_control_op = op;

  m_morph_plan->emit_plan_changed();
}

double
MorphLinear::morphing()
{
  return m_morphing;
}

void
MorphLinear::set_morphing (double new_morphing)
{
  m_morphing = new_morphing;

  m_morph_plan->emit_plan_changed();
}

MorphLinear::ControlType
MorphLinear::control_type()
{
  return m_control_type;
}

void
MorphLinear::set_control_type (ControlType new_control_type)
{
  m_control_type = new_control_type;

  m_morph_plan->emit_plan_changed();
}

bool
MorphLinear::db_linear()
{
  return m_db_linear;
}

void
MorphLinear::set_db_linear (bool dbl)
{
  m_db_linear = dbl;

  m_morph_plan->emit_plan_changed();
}

bool
MorphLinear::use_lpc()
{
  return m_use_lpc;
}

void
MorphLinear::set_use_lpc (bool use_lpc)
{
  m_use_lpc = use_lpc;

  m_morph_plan->emit_plan_changed();
}
