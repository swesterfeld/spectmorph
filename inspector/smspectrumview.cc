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

using std::vector;
using std::max;

SpectrumView::SpectrumView()
{
  time_freq_view_ptr = NULL;
}

static float
value_scale (float value)
{
  if (true)
    {
      double db = bse_db_from_factor (value, -200);
      if (db > -96)
        return db + 96;
      else
        return 0;
    }
  else
    return value;
}

bool
SpectrumView::on_expose_event (GdkEventExpose* ev)
{
  set_size_request (1000, 400);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
  {
    float max_value = 0;
    for (vector<float>::const_iterator mi = spectrum.mags.begin(); mi != spectrum.mags.end(); mi++)
      {
        max_value = max (max_value, value_scale (*mi));
      }

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
    for (size_t i = 0; i < spectrum.mags.size(); i++)
      {
        cr->line_to (double (i) / spectrum.mags.size() * width, height - value_scale (spectrum.mags[i]) / max_value * height);
      }
    //cr->move_to (0, 0);
    //cr->line_to (xc, yc);
    //cr->line_to (0, height);
    //cr->move_to (xc, yc);
    //cr->line_to (width, yc);
    cr->stroke();
  }

  return true;
}

void
SpectrumView::set_spectrum_model (TimeFreqView& tfview)
{
  tfview.signal_spectrum_changed.connect (sigc::mem_fun (*this, &SpectrumView::on_spectrum_changed));
  time_freq_view_ptr = &tfview;
}

void
SpectrumView::on_spectrum_changed()
{
  spectrum = time_freq_view_ptr->get_spectrum();
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}
