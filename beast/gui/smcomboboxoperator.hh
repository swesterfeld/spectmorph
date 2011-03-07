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

#ifndef SPECTMORPH_COMBOBOX_OPERATOR_HH
#define SPECTMORPH_COMBOBOX_OPERATOR_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"

namespace SpectMorph
{

struct OperatorFilter
{
  virtual bool filter (MorphOperator *op) = 0;
};

class ComboBoxOperator : public Gtk::ComboBoxText
{
protected:
  MorphPlan      *morph_plan;
  OperatorFilter *op_filter;
  MorphOperator  *op;
  bool            block_changed;

  void on_operators_changed();
  void on_combobox_changed();

public:
  ComboBoxOperator (MorphPlan *plan, OperatorFilter *op_filter);
  ~ComboBoxOperator();

  MorphOperator *active();
  void set_active (MorphOperator *op);

  sigc::signal<void> signal_active_changed;
};

}

#endif
