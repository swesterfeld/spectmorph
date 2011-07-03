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

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
protected:
  MorphLFO                        *morph_lfo;
  Gtk::Table                       table;

  OperatorFilter                  *operator_filter;

  Gtk::Label                       wave_type_label;
  Gtk::ComboBoxText                wave_type_combobox;

  Gtk::Label                       frequency_label;
  Gtk::HScale                      frequency_scale;
  Gtk::Label                       frequency_value_label;

  Gtk::Label                       depth_label;
  Gtk::HScale                      depth_scale;
  Gtk::Label                       depth_value_label;

  Gtk::Label                       center_label;
  Gtk::HScale                      center_scale;
  Gtk::Label                       center_value_label;

  Gtk::Label                       start_phase_label;
  Gtk::HScale                      start_phase_scale;
  Gtk::Label                       start_phase_value_label;

  void on_wave_type_changed();
  void on_frequency_changed();
  void on_depth_changed();
  void on_center_changed();
  void on_start_phase_changed();
public:
  MorphLFOView (MorphLFO *op, MorphPlanWindow *morph_plan_window);
  ~MorphLFOView();
};

}

#endif
