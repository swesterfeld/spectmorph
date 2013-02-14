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

#include "smmoveindicator.hh"

#include <assert.h>

using namespace SpectMorph;

#if 0
MoveIndicator::MoveIndicator()
{
  set_size_request (-1, 5);
  m_active = false;
}


void
MoveIndicator::set_active (bool active)
{
  m_active = active;

  force_redraw();
}

void
MoveIndicator::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}


bool
MoveIndicator::on_expose_event (GdkEventExpose *event)
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window && m_active)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      cr->rectangle (event->area.x, event->area.y, event->area.width, event->area.height);
      cr->clip();
      cr->set_source_rgb (0, 0, 0.7);
      cr->paint();
    }
  return true;
}
#endif
