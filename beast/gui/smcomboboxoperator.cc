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

#include "smcomboboxoperator.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

ComboBoxOperator::ComboBoxOperator (MorphPlan *morph_plan, OperatorFilter *operator_filter) :
  morph_plan (morph_plan),
  op_filter (operator_filter)
{
  block_changed = false;

  on_operators_changed();

  signal_changed().connect (sigc::mem_fun (*this, &ComboBoxOperator::on_combobox_changed));
}

ComboBoxOperator::~ComboBoxOperator()
{
}

void
ComboBoxOperator::on_operators_changed()
{
  block_changed = true;

  clear();

  const vector<MorphOperator *>& ops = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *morph_op = *oi;
      if (op_filter->filter (morph_op))
        {
          append_text (morph_op->name());
          if (morph_op == op)
            set_active_text (morph_op->name());
        }
    }
  block_changed = false;
}

void
ComboBoxOperator::set_active (MorphOperator *new_op)
{
  op = new_op;

  on_operators_changed();
}

MorphOperator *
ComboBoxOperator::active()
{
  return op;
}

void
ComboBoxOperator::on_combobox_changed()
{
  if (block_changed)
    return;

  op = NULL;

  const vector<MorphOperator *>& ops = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *morph_op = *oi;
      if (morph_op->name() == get_active_text())
        op = morph_op;
    }
  g_printerr ("combobox changed => %s\n", op ? op->name().c_str() : "NONE");
  signal_active_changed();
}
