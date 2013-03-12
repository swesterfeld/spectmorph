// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgrid.hh"
#include "smleakdebugger.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphGrid");

MorphGrid::MorphGrid (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);

  m_width = 1;
  m_height = 1;
  m_selected_x = -1;
  m_selected_y = -1;

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

bool
MorphGrid::save (OutFile& out_file)
{
  return true;
}

bool
MorphGrid::load (InFile& ifile)
{
  return true;
}

MorphOperator::OutputType
MorphGrid::output_type()
{
  return OUTPUT_AUDIO;
}

void
MorphGrid::set_width (int width)
{
  m_width = width;
  update_size();

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::width()
{
  return m_width;
}

void
MorphGrid::set_height (int height)
{
  m_height = height;
  update_size();

  m_morph_plan->emit_plan_changed();
}

int
MorphGrid::height()
{
  return m_height;
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
  m_input_op.resize (m_width);
  for (int i = 0; i < m_width; i++)
    m_input_op[i].resize (m_height);
}

MorphOperator *
MorphGrid::input_op (int x, int y)
{
  g_return_val_if_fail (x >= 0 && x < m_width, NULL);
  g_return_val_if_fail (y >= 0 && y < m_height, NULL);

  return m_input_op[x][y];
}

void
MorphGrid::set_input_op (int x, int y, MorphOperator *op)
{
  g_return_if_fail (x >= 0 && x < m_width);
  g_return_if_fail (y >= 0 && y < m_height);

  m_input_op[x][y] = op;
  m_morph_plan->emit_plan_changed();
}
