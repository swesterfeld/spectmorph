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
#include <bse/bseloader.h>

using namespace SpectMorph;

using std::vector;

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

      cr->save();
      cr->set_source_rgb (1.0, 1.0, 1.0);   // white
      cr->paint();
      cr->restore();

      // red:
      cr->set_source_rgb (1.0, 0.0, 0.0);

      double x = 0;
      double zz = 0.01;
      for (size_t i = 0; i < signal.size(); i++)
        {
          cr->move_to (zz * (i - 1), 200 + x * 200);
          x = signal[i];
          cr->line_to (zz * i, 200 + x * 200);
        }
      cr->stroke();
    }
  return true;
}


void
SampleView::load (GslDataHandle *dhandle, Audio *audio)
{
  signal.clear();

  if (!dhandle) // no sample selected
    return;

  BseErrorType error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "SampleView: can't open the input data handle: %s\n", bse_error_blurb (error));
      exit (1);
    }

  vector<float> block (1024);
  const uint64 n_values = gsl_data_handle_length (dhandle);
  uint64 pos = 0;
  while (pos < n_values)
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      for (uint64 t = 0; t < r; t++)
        signal.push_back (block[t]);
      pos += r;
    }
  gsl_data_handle_close (dhandle);

  force_redraw();
}

void
SampleView::force_redraw()
{
#if 0
  // resize widget according to zoom if necessary
  double scaled_hzoom, scaled_vzoom;
  scale_zoom (&scaled_hzoom, &scaled_vzoom);

  int new_width = image.get_width() * scaled_hzoom;
  int new_height = image.get_height() * scaled_vzoom;
  if (new_width != old_width || new_height != old_height)
    {
      set_size_request (new_width, new_height);
      signal_resized (old_width, old_height, new_width, new_height);

      old_height = new_height;
      old_width = new_width;
    }
#endif

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}


