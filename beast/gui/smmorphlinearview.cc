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

#include "smmorphlinearview.hh"
#include "smmorphplan.hh"
#include <birnet/birnet.hh>

using namespace SpectMorph;

using std::string;
using std::vector;

namespace {

struct MyOperatorFilter : public OperatorFilter
{
  MorphOperator *my_op;

  MyOperatorFilter (MorphOperator *my_op) :
    my_op (my_op)
  {
    //
  }
  bool filter (MorphOperator *op)
  {
    return ((op != my_op) && op->output_type() == MorphOperator::OUTPUT_AUDIO);
  }
};

}

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphLinearView::MorphLinearView (MorphLinear *morph_linear, MainWindow *main_window) :
  MorphOperatorView (morph_linear, main_window),
  morph_linear (morph_linear),
  hscale (-1, 1, 0.01),
  operator_filter (new MyOperatorFilter (morph_linear)),
  left_combobox (morph_linear->morph_plan(), operator_filter),
  right_combobox (morph_linear->morph_plan(), operator_filter)
{
  hscale_label.set_text ("Morphing");
  hscale.set_value (morph_linear->morphing());

  left_combobox.set_active (morph_linear->left_op());
  left_label.set_text ("Left Source");

  table.attach (left_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (left_combobox, 1, 2, 0, 1);

  right_combobox.set_active (morph_linear->right_op());
  right_label.set_text ("Right Source");

  table.attach (right_label, 0, 1, 1, 2, Gtk::SHRINK);
  table.attach (right_combobox, 1, 2, 1, 2);

  control_type_combobox.append_text (CONTROL_TEXT_GUI);
  control_type_combobox.append_text (CONTROL_TEXT_1);
  control_type_combobox.append_text (CONTROL_TEXT_2);
  control_type_label.set_text ("Control Input");

  if (morph_linear->control_type() == MorphLinear::CONTROL_GUI)
    control_type_combobox.set_active_text (CONTROL_TEXT_GUI);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_1)
    control_type_combobox.set_active_text (CONTROL_TEXT_1);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_2)
    control_type_combobox.set_active_text (CONTROL_TEXT_2);
  else
    {
      assert (false);
    }
  update_slider();

  table.attach (control_type_label, 0, 1, 2, 3, Gtk::SHRINK);
  table.attach (control_type_combobox, 1, 2, 2, 3);

  table.attach (hscale_label, 0, 1, 3, 4, Gtk::SHRINK);
  table.attach (hscale, 1, 2, 3, 4);

  table.set_spacings (10);
  table.set_border_width (5);

  frame.add (table);

  left_combobox.signal_active_changed.connect (sigc::mem_fun (*this, &MorphLinearView::on_operator_changed));
  right_combobox.signal_active_changed.connect (sigc::mem_fun (*this, &MorphLinearView::on_operator_changed));
  control_type_combobox.signal_changed().connect (sigc::mem_fun (*this, &MorphLinearView::on_control_type_changed));
  hscale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLinearView::on_morphing_changed));

  show_all_children();
}

MorphLinearView::~MorphLinearView()
{
}

void
MorphLinearView::on_operator_changed()
{
  morph_linear->set_left_op (left_combobox.active());
  morph_linear->set_right_op (right_combobox.active());
}

void
MorphLinearView::on_morphing_changed()
{
  morph_linear->set_morphing (hscale.get_value());
}

void
MorphLinearView::on_control_type_changed()
{
  string text = control_type_combobox.get_active_text();

  if (text == CONTROL_TEXT_GUI)
    morph_linear->set_control_type (MorphLinear::CONTROL_GUI);
  else if (text == CONTROL_TEXT_1)
    morph_linear->set_control_type (MorphLinear::CONTROL_SIGNAL_1);
  else if (text == CONTROL_TEXT_2)
    morph_linear->set_control_type (MorphLinear::CONTROL_SIGNAL_2);
  else
    {
      assert (false);
    }
  update_slider();
}

void
MorphLinearView::update_slider()
{
  bool gui = (morph_linear->control_type() == MorphLinear::CONTROL_GUI);
  hscale.set_sensitive (gui);
}
