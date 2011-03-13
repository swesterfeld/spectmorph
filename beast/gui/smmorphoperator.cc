/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphoperator.hh"
#include "smmorphplan.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOperator::MorphOperator (MorphPlan *morph_plan) :
  m_morph_plan (morph_plan)
{
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

void
MorphOperator::post_load()
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
#include "smmorphsource.hh"

MorphOperator *
MorphOperator::create (const string& type, MorphPlan *plan)
{
  g_return_val_if_fail (plan != NULL, NULL);

  if (type == "SpectMorph::MorphSource") return new MorphSource (plan);
  if (type == "SpectMorph::MorphLinear") return new MorphLinear (plan);
  if (type == "SpectMorph::MorphOutput") return new MorphOutput (plan);

  return NULL;
}
