// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperator.hh"
#include "smmorphplan.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;

MorphOperator::MorphOperator (MorphPlan *morph_plan) :
  m_morph_plan (morph_plan)
{
  m_folded = false;
}

MorphOperator::~MorphOperator()
{
  // virtual destructor for proper subclass deletion
}

MorphPlan*
MorphOperator::morph_plan()
{
  return m_morph_plan;
}

string
MorphOperator::name()
{
  return m_name;
}

void
MorphOperator::set_name (const string& name)
{
  g_return_if_fail (can_rename (name));

  m_name = name;

  m_morph_plan->emit_plan_changed();
}

string
MorphOperator::id()
{
  return m_id;
}

void
MorphOperator::set_id (const string& id)
{
  m_id = id;
}

bool
MorphOperator::folded() const
{
  return m_folded;
}

void
MorphOperator::set_folded (bool folded)
{
  m_folded = folded;

  m_morph_plan->emit_plan_changed();
}

void
MorphOperator::post_load (OpNameMap& op_name_map)
{
  // override this for post load notification
}

void
MorphOperator::write_operator (OutFile& file, const std::string& name, MorphOperator *op)
{
  string op_name;

  if (op) // (op == NULL) => (op_name == "")
    op_name = op->name();

  file.write_string (name, op_name);
}

bool
MorphOperator::can_rename (const string& name)
{
  const vector<MorphOperator *>& ops = m_morph_plan->operators();

  if (name == "")
    return false;

  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *op = *oi;
      if (op != this && op->name() == name)
        return false;
    }
  return true;
}

string
MorphOperator::type_name()
{
  return string (type()).substr (string ("SpectMorph::Morph").size());
}

#include "smmorphoutput.hh"
#include "smmorphlinear.hh"
#include "smmorphgrid.hh"
#include "smmorphsource.hh"
#include "smmorphwavsource.hh"
#include "smmorphlfo.hh"

MorphOperator *
MorphOperator::create (const string& type, MorphPlan *plan)
{
  g_return_val_if_fail (plan != NULL, NULL);

  if (type == "SpectMorph::MorphSource") return new MorphSource (plan);
  if (type == "SpectMorph::MorphWavSource") return new MorphWavSource (plan);
  if (type == "SpectMorph::MorphLinear") return new MorphLinear (plan);
  if (type == "SpectMorph::MorphGrid") return new MorphGrid (plan);
  if (type == "SpectMorph::MorphOutput") return new MorphOutput (plan);
  if (type == "SpectMorph::MorphLFO")    return new MorphLFO (plan);

  return NULL;
}
