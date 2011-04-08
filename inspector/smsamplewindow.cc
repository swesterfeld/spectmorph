/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smsamplewindow.hh"
#include "smnavigator.hh"

#include <iostream>

using namespace SpectMorph;

SampleWindow::SampleWindow (Navigator *navigator) :
  navigator (navigator),
  zoom_controller (1, 5000, 10, 5000)
{
  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("SampleMenu", "Sample"));
  ref_action_group->add (Gtk::Action::create ("SampleNext", "Next Sample"), Gtk::AccelKey ('n', Gdk::ModifierType (0)),
                         sigc::mem_fun (*this, &SampleWindow::on_next_sample));

  ref_ui_manager = Gtk::UIManager::create();
  ref_ui_manager-> insert_action_group (ref_action_group);
  add_accel_group (ref_ui_manager->get_accel_group());

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='SampleMenu'>"
    "      <menuitem action='SampleNext' />"
    "    </menu>"
    "  </menubar>"
    "</ui>";
  try
    {
      ref_ui_manager->add_ui_from_string (ui_info);
    }
  catch (const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " << ex.what();
    }

  Gtk::Widget *menu_bar = ref_ui_manager->get_widget ("/MenuBar");
  if (menu_bar)
    vbox.pack_start (*menu_bar, Gtk::PACK_SHRINK);

  set_border_width (10);
  set_default_size (800, 600);
  set_title ("Sample View");

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  vbox.pack_start (button_hbox, Gtk::PACK_SHRINK);

  button_hbox.add (edit_start_marker);
  button_hbox.add (edit_loop_start);
  button_hbox.add (edit_loop_end);

  edit_start_marker.set_label ("Edit Start Marker");
  edit_start_marker.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &SampleWindow::on_edit_marker_changed),
                                                          SampleView::MARKER_START));
  edit_loop_start.set_label ("Edit Loop Start");
  edit_loop_start.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &SampleWindow::on_edit_marker_changed),
                                                        SampleView::MARKER_LOOP_START));
  edit_loop_end.set_label ("Edit Loop End");
  edit_loop_end.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &SampleWindow::on_edit_marker_changed),
                                                      SampleView::MARKER_LOOP_END));

  add (vbox);
  scrolled_win.add (m_sample_view);
  scrolled_win.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &SampleWindow::on_zoom_changed));
  m_sample_view.signal_resized.connect (sigc::mem_fun (*this, &SampleWindow::on_resized));

  show_all_children();

  in_update_buttons = false;
}

void
SampleWindow::on_dhandle_changed()
{
  load (navigator->get_dhandle(), navigator->get_audio());
}

void
SampleWindow::load (GslDataHandle *dhandle, Audio *audio)
{
  m_sample_view.load (dhandle, audio);
}

void
SampleWindow::on_zoom_changed()
{
  m_sample_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
SampleWindow::on_resized (int old_width, int new_width)
{
  if (old_width > 0 && new_width > 0)
    {
      Gtk::Viewport *view_port = dynamic_cast<Gtk::Viewport*> (scrolled_win.get_child());
      Gtk::Adjustment *hadj = scrolled_win.get_hadjustment();

      const int w_2 = view_port->get_width() / 2;

      hadj->set_value ((hadj->get_value() + w_2) / old_width * new_width - w_2);
    }
}

SampleView&
SampleWindow::sample_view()
{
  return m_sample_view;
}

void
SampleWindow::on_next_sample()
{
  signal_next_sample();
}

void
SampleWindow::on_edit_marker_changed (SampleView::EditMarkerType marker_type)
{
  if (in_update_buttons)
    return;

  if (m_sample_view.edit_marker_type() == marker_type)  // we're selected already -> turn it off
    marker_type = SampleView::MARKER_NONE;

  m_sample_view.set_edit_marker_type (marker_type);

  in_update_buttons = true;
  edit_start_marker.set_active (marker_type == SampleView::MARKER_START);
  edit_loop_start.set_active (marker_type == SampleView::MARKER_LOOP_START);
  edit_loop_end.set_active (marker_type == SampleView::MARKER_LOOP_END);
  in_update_buttons = false;
}
