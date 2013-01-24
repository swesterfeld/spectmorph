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

MorphLinearView::MorphLinearView (MorphLinear *morph_linear, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_linear, morph_plan_window)
{
}

#if 0
namespace {

struct MyOperatorFilter : public OperatorFilter
{
  MorphOperator *my_op;
  MorphOperator::OutputType type;

  MyOperatorFilter (MorphOperator *my_op, MorphOperator::OutputType type) :
    my_op (my_op),
    type (type)
  {
    //
  }
  bool filter (MorphOperator *op)
  {
    return ((op != my_op) && op->output_type() == type);
  }
};

}

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphLinearView::MorphLinearView (MorphLinear *morph_linear, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_linear, morph_plan_window),
  morph_linear (morph_linear),
  hscale (-1, 1, 0.01),
  operator_filter (new MyOperatorFilter (morph_linear, MorphOperator::OUTPUT_AUDIO)),
  control_operator_filter (new MyOperatorFilter (morph_linear, MorphOperator::OUTPUT_CONTROL)),
  left_combobox (morph_linear->morph_plan(), operator_filter),
  right_combobox (morph_linear->morph_plan(), operator_filter),
  control_combobox (morph_linear->morph_plan(), control_operator_filter)
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

  control_combobox.add_str_choice (CONTROL_TEXT_GUI);
  control_combobox.add_str_choice (CONTROL_TEXT_1);
  control_combobox.add_str_choice (CONTROL_TEXT_2);
  control_label.set_text ("Control Input");

  if (morph_linear->control_type() == MorphLinear::CONTROL_GUI)
    control_combobox.set_active_str_choice (CONTROL_TEXT_GUI);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_1)
    control_combobox.set_active_str_choice (CONTROL_TEXT_1);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_2)
    control_combobox.set_active_str_choice (CONTROL_TEXT_2);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_OP)
    control_combobox.set_active (morph_linear->control_op());
  else
    {
      assert (false);
    }
  update_slider();

  db_linear_check_button.set_active (morph_linear->db_linear());
  db_linear_check_button.set_label ("dB Linear Morphing");

  use_lpc_check_button.set_active (morph_linear->use_lpc());
  use_lpc_check_button.set_label ("Use LPC Envelope");

  table.attach (control_label, 0, 1, 2, 3, Gtk::SHRINK);
  table.attach (control_combobox, 1, 2, 2, 3);

  table.attach (hscale_label, 0, 1, 3, 4, Gtk::SHRINK);
  table.attach (hscale, 1, 2, 3, 4);

  table.attach (db_linear_check_button, 0, 2, 4, 5);
  table.attach (use_lpc_check_button, 0, 2, 5, 6);

  table.set_spacings (10);
  table.set_border_width (5);

  frame.add (table);

  left_combobox.signal_active_changed.connect (sigc::mem_fun (*this, &MorphLinearView::on_operator_changed));
  right_combobox.signal_active_changed.connect (sigc::mem_fun (*this, &MorphLinearView::on_operator_changed));
  control_combobox.signal_active_changed.connect (sigc::mem_fun (*this, &MorphLinearView::on_control_changed));
  hscale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLinearView::on_morphing_changed));
  db_linear_check_button.signal_toggled().connect (sigc::mem_fun (*this, &MorphLinearView::on_db_linear_changed));
  use_lpc_check_button.signal_toggled().connect (sigc::mem_fun (*this, &MorphLinearView::on_use_lpc_changed));

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
MorphLinearView::on_control_changed()
{
  MorphOperator *op = control_combobox.active();
  if (op)
    {
      morph_linear->set_control_op (op);
      morph_linear->set_control_type (MorphLinear::CONTROL_OP);
    }
  else
    {
      string text = control_combobox.active_str_choice();

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
    }
  update_slider();
}

void
MorphLinearView::on_db_linear_changed()
{
  morph_linear->set_db_linear (db_linear_check_button.get_active());
}

void
MorphLinearView::on_use_lpc_changed()
{
  morph_linear->set_use_lpc (use_lpc_check_button.get_active());
}

void
MorphLinearView::update_slider()
{
  bool gui = (morph_linear->control_type() == MorphLinear::CONTROL_GUI);
  hscale.set_sensitive (gui);
}
#endif
