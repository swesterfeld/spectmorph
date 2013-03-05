/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include "smlpcview.hh"
#include "smlpc.hh"

#include <QPainter>

using namespace SpectMorph;

using std::vector;
using std::complex;
using std::max;

LPCView::LPCView()
{
  time_freq_view_ptr = NULL;
  hzoom = 1;
  vzoom = 1;
  update_size();
}

void
LPCView::update_size()
{
  resize (600 * hzoom, 600 * vzoom);
}

void
LPCView::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);

  painter.fillRect (rect(), QColor (255, 255, 255));

  const int width =  600 * hzoom;
  const int height = 600 * vzoom;

  painter.setPen (QPen (QColor (0, 0, 200), 2));
  int last_x = 0, last_y = 0;
  for (double t = 0; t < 2 * M_PI + 0.2; t += 0.1)
    {
      int x = (sin (t) + 1) / 2 * width;
      int y = (cos (t) + 1) / 2 * width;
      if (t > 0)
        painter.drawLine (last_x, last_y, x, y);
      last_x = x;
      last_y = y;
    }

  // draw lpc zeros
  if (audio_block.lpc_lsf_p.size() == 26 && audio_block.lpc_lsf_q.size() == 26)
    {
      vector<double> lpc;
      LPC::lsf2lpc (audio_block.lpc_lsf_p, audio_block.lpc_lsf_q, lpc);

      vector< complex<double> > roots;
      LPC::find_roots (lpc, roots);

      painter.setPen (QColor (200, 0, 0));
      for (size_t i = 0; i < roots.size(); i++)
        {
          double root_x = (roots[i].real() + 1) / 2 * width;
          double root_y = (roots[i].imag() + 1) / 2 * width;

          painter.drawLine (root_x - 5, root_y - 5, root_x + 5, root_y + 5);
          painter.drawLine (root_x + 5, root_y - 5, root_x - 5, root_y + 5);
        }

#if 0
        double max_lpc_value = 0;
        for (float freq = 0; freq < M_PI; freq += 0.001)
          {
            double value = env.eval (freq);
            double value_db = bse_db_from_factor (value, -200);
            max_lpc_value = max (max_lpc_value, value_db);
          }
        for (float freq = 0; freq < M_PI; freq += 0.001)
          {
            double value = env.eval (freq);
            double value_db = bse_db_from_factor (value, -200) - max_lpc_value + max_value;
            cr->line_to (freq / M_PI * width, height - value_db / max_value * height);
          }
#endif
        }
}

void
LPCView::set_lpc_model (TimeFreqView *tfview)
{
  connect (tfview, SIGNAL (spectrum_changed()), this, SLOT (on_lpc_changed()));
  time_freq_view_ptr = tfview;
}

void
LPCView::on_lpc_changed()
{
  audio_block = AudioBlock(); // reset

  Audio *audio = time_freq_view_ptr->audio();
  if (audio)
    {
      int frame = time_freq_view_ptr->position_frac() * audio->contents.size();
      int frame_count = audio->contents.size();

      if (frame >= 0 && frame < frame_count)
        {
          audio_block = audio->contents[frame];
        }
    }
  update();
}

#if 0
void
LPCView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}
  
void
LPCView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}
#endif
