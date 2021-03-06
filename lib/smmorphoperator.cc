// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperator.hh"
#include "smmorphplan.hh"
#include "smproperty.hh"
#include "smmodulationlist.hh"
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
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

XParamProperty *
MorphOperator::add_property_xparam (float *value, const string& identifier, const string& label, const string& value_label, float def, float mn, float mx, float slope)
{
  assert (!m_properties[identifier]);
  XParamProperty *property = new XParamProperty (value, identifier, label, value_label, def, mn, mx, slope);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

LinearProperty *
MorphOperator::add_property (float *value, const string& identifier, const string& label, const string& value_label, float def, float mn, float mx)
{
  assert (!m_properties[identifier]);
  LinearProperty *property = new LinearProperty (value, identifier, label, value_label, def, mn, mx);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

IntProperty *
MorphOperator::add_property (int *value, const std::string& identifier,
                             const std::string& label, const std::string& value_label,
                             int def, int mn, int mx)
{
  assert (!m_properties[identifier]);
  IntProperty *property = new IntProperty (value, identifier, label, value_label, def, mn, mx);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

BoolProperty *
MorphOperator::add_property (bool *value, const std::string& identifier,
                             const std::string& label,
                             bool def)
{
  assert (!m_properties[identifier]);
  BoolProperty *property = new BoolProperty (value, identifier, label, def);
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}

EnumProperty *
MorphOperator::add_property_enum (const std::string& identifier,
                                  const std::string& label, int def, const EnumInfo& ei,
                                  std::function<int()> read_func,
                                  std::function<void(int)> write_func)
{
  assert (!m_properties[identifier]);
  EnumProperty *property = new EnumProperty (identifier, label, def, ei, read_func, write_func);
  /* FIXME: FILTER: all the common code for adding properties should be centralized */
  m_properties[identifier].reset (property);
  connect (property->signal_value_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  connect (property->signal_modulation_changed, [this]() { m_morph_plan->emit_plan_changed(); });
  return property;
}


void
MorphOperator::write_properties (OutFile& out_file)
{
  for (auto& kv : m_properties)
    {
      Property *p = kv.second.get();
      p->save (out_file);

      ModulationList *mod_list = p->modulation_list();
      if (mod_list)
        mod_list->save (out_file);
    }
}

bool
MorphOperator::read_property_event (InFile& in_file)
{
  for (auto& kv : m_properties)
    {
      Property *p = kv.second.get();
      if (p->load (in_file))
        return true;

      ModulationList *mod_list = p->modulation_list();
      if (mod_list)
        if (mod_list->load (in_file))
          return true;
    }
  return false;
}

void
MorphOperator::read_properties_post_load (OpNameMap& op_name_map)
{
  for (auto& kv : m_properties)
    {
      Property *p = kv.second.get();

      ModulationList *mod_list = p->modulation_list();
      if (mod_list)
        mod_list->post_load (op_name_map);
    }
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
