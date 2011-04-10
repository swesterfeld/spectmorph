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

#ifndef SPECTMORPH_SAMPLE_WINDOW_HH
#define SPECTMORPH_SAMPLE_WINDOW_HH

#include <gtkmm.h>
#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include "smwavset.hh"

namespace SpectMorph {

class Navigator;
class SampleWindow : public Gtk::Window
{
  Navigator          *navigator;

  Glib::RefPtr<Gtk::UIManager>    ref_ui_manager;
  Glib::RefPtr<Gtk::ActionGroup>  ref_action_group;

  Gtk::ScrolledWindow scrolled_win;
  SampleView          m_sample_view;
  ZoomController      zoom_controller;
  Gtk::HBox           button_hbox;

  Gtk::ToggleButton   edit_start_marker;
  Gtk::ToggleButton   edit_loop_start;
  Gtk::ToggleButton   edit_loop_end;
  Gtk::ComboBoxText   loop_type_combo;
  bool                in_update_buttons;

  Gtk::VBox           vbox;
public:
  SampleWindow (Navigator *navigator);

  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio);
  SampleView& sample_view();

  void on_dhandle_changed();
  void on_zoom_changed();
  void on_resized (int old_width, int new_width);
  void on_next_sample();
  void on_edit_marker_changed (SampleView::EditMarkerType marker_type);
  void on_loop_type_changed();

  sigc::signal<void> signal_next_sample;
};

}

#endif
