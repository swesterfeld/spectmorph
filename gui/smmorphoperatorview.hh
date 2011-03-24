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

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include <gtkmm.h>
#include "smmorphplanwindow.hh"

namespace SpectMorph
{

class MorphOperator;
class MorphOperatorView : public Gtk::EventBox
{
protected:
  bool              in_move;

  Gtk::VBox         vbox;
  Gtk::Frame        frame;
  MorphPlanWindow  *morph_plan_window;
  MorphOperator    *m_op;

  Gdk::Cursor    move_cursor;

  void on_operators_changed();
public:
  MorphOperatorView (MorphOperator *op, MorphPlanWindow *morph_plan_window);

  MorphOperator *op();

  bool on_button_press_event (GdkEventButton *event);
  bool on_motion_notify_event (GdkEventMotion *event);
  bool on_button_release_event (GdkEventButton *event);

  sigc::signal<void, MorphOperator *> signal_move_indication;

  static MorphOperatorView *create (MorphOperator *op, MorphPlanWindow *window);
};

}

#endif
