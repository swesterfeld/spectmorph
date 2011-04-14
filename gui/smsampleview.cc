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
using std::min;
using std::max;

#define HZOOM_SCALE 0.05

SampleView::SampleView() :
  audio (NULL),
  markers (NULL)
{
  set_size_request (400, 400);

  add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);

  attack_start = 0;
  attack_end = 0;
  hzoom = 1;
  vzoom = 1;
  old_width = -1;
  m_edit_marker_type = MARKER_NONE;
  button_1_pressed = false;
}

bool
SampleView::on_expose_event (GdkEventExpose *ev)
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
    {
      Gtk::Allocation allocation = get_allocation();
      const int width = allocation.get_width();
      const int height = allocation.get_height();

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

      // red sample:
      cr->set_source_rgb (0.8, 0.0, 0.0);

      double hz = HZOOM_SCALE * hzoom;
      double vz = (height / 2) * vzoom;
      draw_signal (signal, cr, ev, height, vz, hz);
      cr->stroke();

      // attack markers:
      cr->set_source_rgb (0.6, 0.6, 0.6);
      cr->move_to (hz * attack_start, 0);
      cr->line_to (hz * attack_start, height);

      cr->move_to (hz * attack_end, 0);
      cr->line_to (hz * attack_end, height);
      cr->stroke();

      if (audio)
        {
          // start marker
          int start = audio->start_ms / 1000.0 * audio->mix_freq - audio->zero_values_at_start;

          if (edit_marker_type() == MARKER_START)
            cr->set_source_rgb (0, 0, 0.8);
          else
            cr->set_source_rgb (0.6, 0.6, 0.6);

          cr->move_to (hz * start, 0);
          cr->line_to (hz * start, height);
          cr->stroke();

          if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
            {
              // loop start marker
              int loop_start = audio->loop_start * audio->frame_step_ms / 1000.0 * audio->mix_freq;

              if (edit_marker_type() == MARKER_LOOP_START)
                cr->set_source_rgb (0, 0, 0.8);
              else
                cr->set_source_rgb (0.6, 0.6, 0.6);

              cr->move_to (hz * loop_start, 0);
              cr->line_to (hz * loop_start, height);
              cr->stroke();

              // loop end marker
              int loop_end = audio->loop_end * audio->frame_step_ms / 1000.0 * audio->mix_freq;

              if (edit_marker_type() == MARKER_LOOP_END)
                cr->set_source_rgb (0, 0, 0.8);
              else
                cr->set_source_rgb (0.6, 0.6, 0.6);

              cr->move_to (hz * loop_end, 0);
              cr->line_to (hz * loop_end, height);
              cr->stroke();
            }
        }
      if (markers)
        {
          for (size_t i = 0; i < markers->count(); i++)
            {
              int marker_pos = markers->position (i) / 1000.0 * audio->mix_freq;

              if (markers->type (i) == m_edit_marker_type)
                cr->set_source_rgb (0, 0, 0.8);
              else
                cr->set_source_rgb (0.6, 0.6, 0.6);

              cr->move_to (hz * marker_pos, 0);
              cr->line_to (hz * marker_pos, height);
              cr->stroke();
            }
        }

      // dark blue line @ zero:
      cr->set_source_rgb (0.0, 0.0, 0.0);
      cr->move_to (0, (height / 2));
      cr->line_to (width, (height / 2));
      cr->stroke();
    }
  return true;
}

bool
SampleView::on_button_press_event (GdkEventButton *event)
{
  if (event->button == 1)
    button_1_pressed = true;

  move_marker (event->x);
  return true;
}

void
SampleView::move_marker (int x)
{
  if (button_1_pressed && audio)
    {
      double hz = HZOOM_SCALE * hzoom;
      int index = x / hz;

      if (m_edit_marker_type == MARKER_START)
        {
          audio->start_ms = (index + audio->zero_values_at_start) / audio->mix_freq * 1000;
        }
      if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
        {
          if (m_edit_marker_type == MARKER_LOOP_START)
            {
              audio->loop_start = index / (audio->frame_step_ms / 1000 * audio->mix_freq);
            }
          else if (m_edit_marker_type == MARKER_LOOP_END)
            {
              audio->loop_end = index / (audio->frame_step_ms / 1000 * audio->mix_freq);
            }
        }
      if (markers)
        {
          for (size_t m = 0; m < markers->count(); m++)
            {
              if (markers->type (m) == m_edit_marker_type)
                markers->set_position (m, index / audio->mix_freq * 1000);
            }
        }
      signal_audio_edit();
      force_redraw();
    }
}

bool
SampleView::on_motion_notify_event (GdkEventMotion *event)
{
  if (audio)
    {
      double hz = HZOOM_SCALE * hzoom;
      int index = event->x / hz;

      signal_mouse_time_changed (index / audio->mix_freq * 1000);
    }
  move_marker (event->x);
  return true;
}

bool
SampleView::on_button_release_event (GdkEventButton *event)
{
  move_marker (event->x);

  if (event->button == 1)
    button_1_pressed = false;
  return true;
}

void
SampleView::load (GslDataHandle *dhandle, Audio *audio, Markers *markers)
{
  this->audio = audio;
  this->markers = markers;

  signal.clear();
  attack_start = 0;
  attack_end = 0;

  if (!dhandle) // no sample selected
    {
      force_redraw();
      return;
    }

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

  old_width = -1;
  force_redraw();
}

void
SampleView::force_redraw()
{
  int new_width = HZOOM_SCALE * signal.size() * hzoom;
  if (new_width != old_width);
    {
      set_size_request (new_width, -1);
      signal_resized (old_width, new_width);

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

SampleView::EditMarkerType
SampleView::edit_marker_type()
{
  return m_edit_marker_type;
}

void
SampleView::set_edit_marker_type (EditMarkerType marker_type)
{
  m_edit_marker_type = marker_type;
  force_redraw();
}
