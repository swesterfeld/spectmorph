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

#include "smsampleview.hh"

using namespace SpectMorph;

SampleView::SampleView()
{
  set_size_request (400, 400);
}

bool
SampleView::on_expose_event (GdkEventExpose *ev)
{
  // draw contents
  //Glib::RefPtr<Gdk::Pixbuf> zimage;
  //zimage = zoom_rect (image, ev->area.x, ev->area.y, ev->area.width, ev->area.height, scaled_hzoom, scaled_vzoom, position,
   //                   display_min_db, display_boost);
  //zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, ev->area.x, ev->area.y,
   //                           zimage->get_width(), zimage->get_height(),
    //                          Gdk::RGB_DITHER_NONE, 0, 0);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      cr->set_line_width (1.0);

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      cr->clip();

      // red:
      cr->set_source_rgb (1.0, 0.0, 0.0);

      cr->move_to (0, 0);
      cr->line_to (1, 1);
      cr->stroke();
    }
  return true;
}


