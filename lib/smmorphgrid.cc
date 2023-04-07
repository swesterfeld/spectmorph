// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphgrid.hh"
#include "smleakdebugger.hh"
#include "smmorphplan.hh"
#include "smwavset.hh"
#include "smwavsetrepo.hh"
#include "smutils.hh"
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::u32string;
using std::map;
using std::pair;
using std::make_pair;

static LeakDebugger leak_debugger ("SpectMorph::MorphGrid");

MorphGrid::MorphGrid (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

  connect (morph_plan->signal_operator_removed, this, &MorphGrid::on_operator_removed);

  m_config.width = 2;
  m_config.height = 1;
  m_zoom = 5;
  m_selected_x = -1;
  m_selected_y = -1;
  auto x_morphing = add_property (&m_config.x_morphing_mod, P_X_MORPHING, "X Morphing", "%.2f", 0, -1, 1);
  auto y_morphing = add_property (&m_config.y_morphing_mod, P_Y_MORPHING, "Y Morphing", "%.2f", 0, -1, 1);
  x_morphing->modulation_list()->set_compat_type_and_op ("x_control_type", "x_control_op");
  y_morphing->modulation_list()->set_compat_type_and_op ("y_control_type", "y_control_op");

  update_size();
}

MorphGrid::~MorphGrid()
{
  leak_debugger.del (this);
}

const char *
MorphGrid::type()
{
  return "SpectMorph::MorphGrid";
}

int
MorphGrid::insert_order()
{
  return 700;
}

bool
MorphGrid::save (OutFile& out_file)
{
  write_properties (out_file);

  out_file.write_int ("width", m_config.width);
  out_file.write_int ("height", m_config.height);
  out_file.write_int ("zoom", m_zoom);

  for (int x = 0; x < m_config.width; x++)
    {
      for (int y = 0; y < m_config.height; y++)
        {
          string op_name = string_printf ("input_op_%d_%d", x, y);
          string delta_db_name = string_printf ("input_delta_db_%d_%d", x, y);
          string smset_name = string_printf ("input_smset_%d_%d", x, y);

          out_file.write_operator (op_name, m_config.input_node[x][y].op);
          out_file.write_float (delta_db_name, m_config.input_node[x][y].delta_db);
          out_file.write_string (smset_name, m_config.input_node[x][y].smset);
        }
    }

  return true;
}

bool
MorphGrid::load (InFile& ifile)
{
  map<string, pair<int, int> > delta_db_map;

  load_map.clear();

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile))
        {
          // property has been read, so we ignore the event
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "width")
            {
              m_config.width = ifile.event_int();
              update_size();
            }
          else if (ifile.event_name() == "height")
            {
              m_config.height = ifile.event_int();
              update_size();
            }
          else if (ifile.event_name() == "zoom")
            {
              m_zoom = ifile.event_int();
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (delta_db_map.empty())
            {
              for (int x = 0; x < m_config.width; x++)
                {
                  for (int y = 0; y < m_config.height; y++)
                    {
                      string name = string_printf ("input_delta_db_%d_%d", x, y);

                      delta_db_map[name] = make_pair (x, y);
                    }
                }
            }
          map<string, pair<int, int> >::const_iterator it = delta_db_map.find (ifile.event_name());
          if (it == delta_db_map.end())
            {
              g_printerr ("bad float\n");
              return false;
            }
          const int x = it->second.first;
          const int y = it->second.second;
          m_config.input_node[x][y].delta_db = ifile.event_float();
        }
      else if (ifile.event() == InFile::STRING)
        {
          // we need to wait until all other operators have been loaded
          // before we can put a pointer in the m_input_op data structure
          load_map[ifile.event_name()] = ifile.event_data();
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
MorphGrid::post_load (OpNameMap& op_name_map)
{
  for (int x = 0; x < m_config.width; x++)
    {
      for (int y = 0; y < m_config.height; y++)
        {
          string name = string_printf ("input_op_%d_%d", x, y);
          string smset_name = string_printf ("input_smset_%d_%d", x, y);

          m_config.input_node[x][y].op.set (op_name_map[load_map[name]]);
          m_config.input_node[x][y].smset = load_map[smset_name];
        }
    }
}


MorphOperator::OutputType
MorphGrid::output_type()
{
  return OUTPUT_AUDIO;
}

void
MorphGrid::set_width (int width)
{
  m_config.width = width;
  update_size();

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::width()
{
  return m_config.width;
}

void
MorphGrid::set_height (int height)
{
  m_config.height = height;
  update_size();

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::height()
{
  return m_config.height;
}

void
MorphGrid::set_selected_x (int x)
{
  m_selected_x = x;

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::selected_x()
{
  return m_selected_x;
}

void
MorphGrid::set_selected_y (int y)
{
  m_selected_y = y;

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::selected_y()
{
  return m_selected_y;
}

bool
MorphGrid::has_selection()
{
  return m_selected_x >= 0 && m_selected_y >= 0;
}

void
MorphGrid::update_size()
{
  m_config.input_node.resize (m_config.width);
  for (int i = 0; i < m_config.width; i++)
    m_config.input_node[i].resize (m_config.height);
}

MorphGridNode
MorphGrid::input_node (int x, int y)
{
  g_return_val_if_fail (x >= 0 && x < m_config.width, MorphGridNode());
  g_return_val_if_fail (y >= 0 && y < m_config.height, MorphGridNode());

  return m_config.input_node[x][y];
}

void
MorphGrid::set_input_node (int x, int y, const MorphGridNode& node)
{
  g_return_if_fail (x >= 0 && x < m_config.width);
  g_return_if_fail (y >= 0 && y < m_config.height);
  g_return_if_fail (node.smset == "" || !node.op);  // should not set both

  m_config.input_node[x][y] = node;
  m_morph_plan->emit_plan_changed();
}

void
MorphGrid::set_zoom (int z)
{
  m_zoom = z;
  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::zoom()
{
  return m_zoom;
}

double
MorphGrid::x_morphing()
{
  return property (P_X_MORPHING)->get_float();
}

void
MorphGrid::set_x_morphing (double new_morphing)
{
  /* smrunplan test */
  property (P_X_MORPHING)->set_float (new_morphing);
}

double
MorphGrid::y_morphing()
{
  return property (P_Y_MORPHING)->get_float();
}

void
MorphGrid::set_y_morphing (double new_morphing)
{
  property (P_Y_MORPHING)->set_float (new_morphing);
}

void
MorphGrid::on_operator_removed (MorphOperator *op)
{
  // plan changed will be emitted automatically after remove, so we don't emit it here

  for (int x = 0; x < m_config.width; x++)
    {
      for (int y = 0; y < m_config.height; y++)
        {
          if (m_config.input_node[x][y].op.get() == op)
            {
              assert (m_config.input_node[x][y].smset.empty());

              m_config.input_node[x][y].op.set (nullptr);
            }
        }
    }
}

string
MorphGrid::input_node_label (int x, int y)
{
  g_return_val_if_fail (x >= 0 && x < m_config.width, "XXX");
  g_return_val_if_fail (y >= 0 && y < m_config.height, "XXX");

  const MorphGridNode& node = m_config.input_node[x][y];
  if (node.smset != "")
    {
      string smset_dir = m_morph_plan->index()->smset_dir();
      string path = smset_dir + "/" + node.smset;

      WavSet *wav_set = WavSetRepo::the()->get (path);
      if (wav_set)
        return wav_set->short_name;
    }
  else if (node.op)
    {
      u32string result;
      u32string name = to_utf32 (node.op.get()->name());

      if (name.size() >= 1)
        result += name[0];

      if (name.size() >= 3)
        result += name[name.size() - 2];

      if (name.size() >= 2)
        result += name[name.size() - 1];

      return to_utf8 (result);
    }
  return "???";
}

vector<MorphOperator *>
MorphGrid::dependencies()
{
  std::vector<MorphOperator *> deps;

  get_property_dependencies (deps, { P_X_MORPHING, P_Y_MORPHING });
  for (int x = 0; x < m_config.width; x++)
    {
      for (int y = 0; y < m_config.height; y++)
        {
          deps.push_back (m_config.input_node[x][y].op.get());
        }
    }
  return deps;
}

MorphOperatorConfig *
MorphGrid::clone_config()
{
  Config *cfg = new Config (m_config);

  string smset_dir = morph_plan()->index()->smset_dir();

  for (int x = 0; x < m_config.width; x++)
    {
      for (int y = 0; y < m_config.height; y++)
        {
          string smset = cfg->input_node[x][y].smset;
          if (smset != "")
            {
              cfg->input_node[x][y].wav_set = WavSetRepo::the()->get (smset_dir + "/" + smset);
            }
          else
            {
              cfg->input_node[x][y].wav_set = nullptr;
            }
        }
    }
  return cfg;
}

MorphGridNode::MorphGridNode() :
  delta_db (0)
{
}
