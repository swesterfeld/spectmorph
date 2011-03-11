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

#include "smmorphoperatorview.hh"
#include "smmorphoperator.hh"

using namespace SpectMorph;

MorphOperatorView::MorphOperatorView (MorphOperator *op, MainWindow *main_window) :
  in_move (false),
  main_window (main_window),
  m_op (op)
{
  on_operators_changed();
  move_cursor = Gdk::Cursor (gdk_cursor_new (GDK_FLEUR));

  m_op->morph_plan()->signal_plan_changed.connect (sigc::mem_fun (*this, &MorphOperatorView::on_operators_changed));
  add (frame);
}

bool
MorphOperatorView::on_button_press_event (GdkEventButton *event)
{
  if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      window->set_cursor (move_cursor);

      in_move = true;
      return true;
    }
  else if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      main_window->show_popup (event, m_op);
      return true; // it has been handled
    }
  else
    return false;
}

bool
MorphOperatorView::on_motion_notify_event (GdkEventMotion *event)
{
  if (in_move)
    {
      MorphOperator *op_next = main_window->where (m_op, event->x_root, event->y_root);
      signal_move_indication (op_next);
    }
  return false;
}

bool
MorphOperatorView::on_button_release_event (GdkEventButton *event)
{
  if (event->type == GDK_BUTTON_RELEASE && event->button == 1)
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      window->set_cursor ();
      MorphOperator *op_next = main_window->where (m_op, event->x_root, event->y_root);
      in_move = false;

      // DELETION can occur here
      m_op->morph_plan()->move (m_op, op_next);
      return true;
    }
  return false;
}

void
MorphOperatorView::on_operators_changed()
{
  frame.set_label (m_op->type_name() + ": " + m_op->name());
}

MorphOperator *
MorphOperatorView::op()
{
  return m_op;
}
