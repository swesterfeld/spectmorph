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

#include "smspectrumview.hh"

using namespace SpectMorph;

SpectrumView::SpectrumView()
{
}

bool
SpectrumView::on_expose_event (GdkEventExpose* ev)
{
  set_size_request (1000, 400);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // coordinates for the center of the window
    int xc, yc;
    xc = width / 2;
    yc = height / 2;

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

    cr->save();
    cr->set_source_rgb (1.0, 1.0, 1.0);   // white
    cr->paint();
    cr->restore();

    cr->set_line_width (2.0);

    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
    cr->clip();

    // draw red lines out from the center of the window
    cr->set_source_rgb (0.8, 0.0, 0.0);
    cr->move_to (0, 0);
    cr->line_to (xc, yc);
    cr->line_to (0, height);
    cr->move_to (xc, yc);
    cr->line_to (width, yc);
    cr->stroke();
  }

  return true;
}
