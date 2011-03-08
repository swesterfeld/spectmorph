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

namespace SpectMorph
{

class MorphLinearView : public MorphOperatorView
{
protected:
  MorphLinear                     *morph_linear;
  Gtk::Table                       table;

  Gtk::Label                       hscale_label;
  Gtk::HScale                      hscale;

  OperatorFilter                  *operator_filter;

  Gtk::Label                       left_label;
  ComboBoxOperator                 left_combobox;

  Gtk::Label                       right_label;
  ComboBoxOperator                 right_combobox;

  void on_operator_changed();
  void on_morphing_changed();

public:
  MorphLinearView (MorphLinear *op, MainWindow *main_window);
  ~MorphLinearView();
};

}

#endif
