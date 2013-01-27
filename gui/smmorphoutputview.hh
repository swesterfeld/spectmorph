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

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"

#include <QComboBox>
#include <QCheckBox>

namespace SpectMorph
{

class MorphOutputView : public MorphOperatorView
{
  Q_OBJECT

  struct ChannelView {
    QLabel           *label;
    ComboBoxOperator *combobox;
  };
  std::vector<ChannelView *>  channels;
  MorphOutput                *morph_output;
public:
  MorphOutputView (MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

public slots:
  void on_sines_changed (bool new_value);
  void on_noise_changed (bool new_value);
  void on_operator_changed();
};

#if 0
class MorphOutputView : public MorphOperatorView
{
  struct ChannelView {
    ChannelView (MorphPlan *plan, OperatorFilter *of) :
      combobox (plan, of)
    {
    }
    Gtk::Label        label;
    ComboBoxOperator  combobox;
  };

  Gtk::Table                  channel_table;
  Gtk::CheckButton            sines_check_button;
  Gtk::CheckButton            noise_check_button;
  std::vector<ChannelView *>  channels;
  MorphOutput                *morph_output;

public:
  MorphOutputView (MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);
  ~MorphOutputView();

  void on_operator_changed();
  void on_sines_changed();
  void on_noise_changed();
};
#endif

}

#endif
