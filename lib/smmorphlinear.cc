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
  connect (morph_plan->signal_operator_removed, this, &MorphLinear::on_operator_removed);

  auto morphing = add_property (&m_config.morphing, P_MORPHING, "Morphing", "%.2f", 0, -1, 1);
  morphing->set_modulation_data (&m_config.morphing_mod);

  m_config.db_linear = false;

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

int
MorphLinear::insert_order()
{
  return 500;
}

bool
MorphLinear::save (OutFile& out_file)
{
  write_operator (out_file, "left", m_config.left_op);
  write_operator (out_file, "right", m_config.right_op);
  out_file.write_string ("left_smset", m_left_smset);
  out_file.write_string ("right_smset", m_right_smset);
  out_file.write_float ("morphing", m_config.morphing);
  out_file.write_bool ("db_linear", m_config.db_linear);

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
          else if (ifile.event_name() == "left_smset")
            {
              m_left_smset = ifile.event_data();
            }
          else if (ifile.event_name() == "right_smset")
            {
              m_right_smset = ifile.event_data();
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
              m_config.morphing = ifile.event_float();
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
              // FIXME: FILTER: m_config.control_type = static_cast<ControlType> (ifile.event_int());
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
              m_config.db_linear = ifile.event_bool();
            }
          else if (ifile.event_name() == "use_lpc")
            {
              /* we need to skip this without error, as old files may contain this setting
               * lpc is however no longer supported, so we always ignore the value
               */
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
  m_config.left_op.set (op_name_map[load_left]);
  m_config.right_op.set (op_name_map[load_right]);
  // FIXME: FILTER m_config.control_op.set (op_name_map[load_control]);
}

MorphOperator::OutputType
MorphLinear::output_type()
{
  return OUTPUT_AUDIO;
}

void
MorphLinear::on_operator_removed (MorphOperator *op)
{
  // plan changed will be emitted automatically after remove, so we don't emit it here

  if (op == m_config.left_op.get())
    m_config.left_op.set (nullptr);

  if (op == m_config.right_op.get())
    m_config.right_op.set (nullptr);
}

MorphOperator *
MorphLinear::left_op()
{
  return m_config.left_op.get();
}

MorphOperator *
MorphLinear::right_op()
{
  return m_config.right_op.get();
}

void
MorphLinear::set_left_op (MorphOperator *op)
{
  m_config.left_op.set (op);

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_right_op (MorphOperator *op)
{
  m_config.right_op.set (op);

  m_morph_plan->emit_plan_changed();
}

string
MorphLinear::left_smset()
{
  return m_left_smset;
}

string
MorphLinear::right_smset()
{
  return m_right_smset;
}

void
MorphLinear::set_left_smset (const string& smset)
{
  m_left_smset = smset;

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_right_smset (const string& smset)
{
  m_right_smset = smset;

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_morphing (double new_morphing)
{
  /* not used by the UI, but by smrunplan test */
  property (P_MORPHING)->set_float (new_morphing);
}

bool
MorphLinear::db_linear()
{
  return m_config.db_linear;
}

void
MorphLinear::set_db_linear (bool dbl)
{
  m_config.db_linear = dbl;

  m_morph_plan->emit_plan_changed();
}

vector<MorphOperator *>
MorphLinear::dependencies()
{
  return {
    m_config.left_op.get(),
    m_config.right_op.get(),
    // FIXME: FILTER m_config.control_type == CONTROL_OP ? m_config.control_op.get() : nullptr
  };
}

MorphOperatorConfig *
MorphLinear::clone_config()
{
  Config *cfg = new Config (m_config);

  string smset_dir = morph_plan()->index()->smset_dir();

  cfg->left_path = "";
  if (m_left_smset != "")
    cfg->left_path = smset_dir + "/" + m_left_smset;

  cfg->right_path = "";
  if (m_right_smset != "")
    cfg->right_path = smset_dir + "/" + m_right_smset;

  return cfg;
}
