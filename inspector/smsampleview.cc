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
  attack_start = 0;
  attack_end = 0;
  hzoom = 1;
  vzoom = 1;
  old_width = -1;
  old_height = -1;
}

bool
SampleView::on_expose_event (GdkEventExpose *ev)
{
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
      cr->set_source_rgb (1.0, 1.0, 1.0);   // white background
      cr->paint();
      cr->restore();

      // blue sample:
      cr->set_source_rgb (0.8, 0.0, 0.0);

      double x = 0;
      double zz = 0.01 * hzoom;
      double vz = 200 * vzoom;
      for (size_t i = 0; i < signal.size(); i++)
        {
          cr->move_to (zz * MAX (0, i - 1), vz + x * vz);
          x = signal[i];
          cr->line_to (zz * i, vz + x * vz);
        }
      cr->stroke();

      // attack markers:
      cr->set_source_rgb (0.6, 0.6, 0.6);
      cr->move_to (zz * attack_start, 0);
      cr->line_to (zz * attack_start, 2 * vz);

      cr->move_to (zz * attack_end, 0);
      cr->line_to (zz * attack_end, 2 * vz);
      cr->stroke();

      // dark blue line @ zero:
      cr->set_source_rgb (0.0, 0.0, 0.0);
      cr->move_to (0, vz);
      cr->line_to (signal.size() * zz, vz);
      cr->stroke();

    }
  return true;
}


void
SampleView::load (GslDataHandle *dhandle, Audio *audio)
{
  signal.clear();
  attack_start = 0;
  attack_end = 0;

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

  attack_start = audio->attack_start_ms / 1000.0 * audio->mix_freq - audio->zero_values_at_start;
  attack_end   = audio->attack_end_ms / 1000.0 * audio->mix_freq - audio->zero_values_at_start;

  force_redraw();
}

void
SampleView::force_redraw()
{
  int new_width = 0.01 * signal.size() * hzoom;
  int new_height = 400 * vzoom;
  if (new_width != old_width || new_height != old_height)
    {
      set_size_request (new_width, new_height);
      signal_resized (old_width, old_height, new_width, new_height);

      old_height = new_height;
      old_width = new_width;
    }

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
SampleView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}
