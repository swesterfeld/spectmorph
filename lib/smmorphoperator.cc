// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperator.hh"
#include "smmorphplan.hh"
#include "smproperty.hh"
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
MorphOperator::write_operator (OutFile& file, const std::string& name, const MorphOperatorPtr& op)
{
  string op_name;

  if (op.get()) // (op == NULL) => (op_name == "")
    op_name = op.get()->name();

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

vector<MorphOperator *>
MorphOperator::dependencies()
{
  return {}; /* default implementation -> no dependencies */
}

Property *
MorphOperator::property (const string& identifier)
{
  return m_properties[identifier].get();
}

LogProperty *
MorphOperator::add_property_log (float *value, const string& identifier, const string& label, const string& value_label, float def, float mn, float mx)
{
  assert (!m_properties[identifier]);
  LogProperty *property = new LogProperty (value, identifier, label, value_label, def, mn, mx);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

XParamProperty *
MorphOperator::add_property_xparam (float *value, const string& identifier, const string& label, const string& value_label, float def, float mn, float mx, float slope)
{
  assert (!m_properties[identifier]);
  XParamProperty *property = new XParamProperty (value, identifier, label, value_label, def, mn, mx, slope);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

LinearProperty *
MorphOperator::add_property (float *value, const string& identifier, const string& label, const string& value_label, float def, float mn, float mx)
{
  assert (!m_properties[identifier]);
  LinearProperty *property = new LinearProperty (value, identifier, label, value_label, def, mn, mx);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

void
MorphOperator::write_properties (OutFile& out_file)
{
  for (auto& kv : m_properties)
    {
      PropertyBase *pb = dynamic_cast<PropertyBase *> (kv.second.get());
      if (pb)
        {
          pb->save (out_file);
        }
    }
}

bool
MorphOperator::read_property_event (InFile& in_file)
{
  for (auto& kv : m_properties)
    {
      PropertyBase *pb = dynamic_cast<PropertyBase *> (kv.second.get());
      if (pb)
        {
          if (pb->load (in_file))
            return true;
        }
    }
  return false;
}

MorphOperatorConfig *
MorphOperator::clone_config()
{
  return nullptr; // FIXME: remove default impl
}

MorphOperatorConfig::~MorphOperatorConfig()
{
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
