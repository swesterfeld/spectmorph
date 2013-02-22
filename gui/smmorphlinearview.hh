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

#ifndef SPECTMORPH_MORPH_LINEAR_VIEW_HH
#define SPECTMORPH_MORPH_LINEAR_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"
#include "smcomboboxoperator.hh"

#include <QLabel>
#include <QSlider>
#include <QStackedWidget>

namespace SpectMorph
{

class MorphLinearView : public MorphOperatorView
{
  Q_OBJECT
protected:
  MorphLinear                     *morph_linear;

  OperatorFilter                  *operator_filter;
  OperatorFilter                  *control_operator_filter;
  QLabel                          *morphing_label;
  QSlider                         *morphing_slider;
  QStackedWidget                  *morphing_stack;

  ComboBoxOperator                *left_combobox;
  ComboBoxOperator                *right_combobox;
  ComboBoxOperator                *control_combobox;

  void update_slider();

public:
  MorphLinearView (MorphLinear *op, MorphPlanWindow *morph_plan_window);

public slots:
  void on_morphing_changed (int new_value);
  void on_control_changed();
  void on_operator_changed();
  void on_db_linear_changed (bool new_value);
  void on_use_lpc_changed (bool new_value);
};

}

#endif
