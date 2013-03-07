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
#include <stdio.h>

#include <QPainter>
#include <QMouseEvent>

using namespace SpectMorph;

using std::vector;
using std::min;
using std::max;

#define HZOOM_SCALE 0.05

SampleView::SampleView()
{
  audio = NULL;
  markers = NULL;
  attack_start = 0;
  attack_end = 0;
  hzoom = 1;
  vzoom = 1;
  m_edit_marker_type = MARKER_NONE;
  button_1_pressed = false;
  update_size();

  setMouseTracking (true);
}

void
SampleView::paintEvent (QPaintEvent *event)
{
  const int width = this->width();
  const int height = this->height();

  QPainter painter (this);
  painter.fillRect (rect(), QColor (255, 255, 255));

  painter.setPen (QColor (200, 0, 0));
  double hz = HZOOM_SCALE * hzoom;
  double vz = (height / 2) * vzoom;
  draw_signal (signal, painter, event->rect(), height, vz, hz);

  // attack markers:
  painter.setPen (QColor (150, 150, 150));
  painter.drawLine (hz * attack_start, 0, hz * attack_start, height);
  painter.drawLine (hz * attack_end, 0, hz * attack_end, height);

  if (audio)
    {
      // start marker
      int start = audio->start_ms / 1000.0 * audio->mix_freq - audio->zero_values_at_start;

      if (edit_marker_type() == MARKER_START)
        painter.setPen (QColor (0, 0, 200));
      else
        painter.setPen (QColor (150, 150, 150));
      painter.drawLine (hz * start, 0, hz * start, height);

      if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
        {
          // loop start marker
          int loop_start = audio->loop_start * audio->frame_step_ms / 1000.0 * audio->mix_freq;

          if (edit_marker_type() == MARKER_LOOP_START)
            painter.setPen (QColor (0, 0, 200));
          else
            painter.setPen (QColor (150, 150, 150));
          painter.drawLine (hz * loop_start, 0, hz * loop_start, height);

          // loop end marker
          int loop_end = audio->loop_end * audio->frame_step_ms / 1000.0 * audio->mix_freq;

          if (edit_marker_type() == MARKER_LOOP_END)
            painter.setPen (QColor (0, 0, 200));
          else
            painter.setPen (QColor (150, 150, 150));
          painter.drawLine (hz * loop_end, 0, hz * loop_end, height);
        }
    }

  if (markers)
    {
      for (size_t i = 0; i < markers->count(); i++)
        {
          if (markers->valid (i))
            {
              int marker_pos = markers->position (i) / 1000.0 * audio->mix_freq;

              if (markers->type (i) == m_edit_marker_type)
                painter.setPen (QColor (0, 0, 200));
              else
                painter.setPen (QColor (150, 150, 150));

              painter.drawLine (hz * marker_pos, 0, hz * marker_pos, height);
            }
        }
    }

  // black line @ zero:
  painter.setPen (QColor (0, 0, 0));
  painter.drawLine (0, (height / 2), width, height / 2);
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
      update_size();
      update();
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

  update_size();
  update();
}

void
SampleView::update_size()
{
  int new_width = HZOOM_SCALE * signal.size() * hzoom;

  setFixedWidth (new_width);
}

void
SampleView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  update_size();
  update();
}

void
SampleView::mousePressEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      button_1_pressed = true;
      if (m_edit_marker_type && audio)
        setCursor (Qt::SizeAllCursor);

      move_marker (event->x());
    }
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
      if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
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
      emit audio_edit();
      update();
    }
}

void
SampleView::mouseMoveEvent (QMouseEvent *event)
{
  if (audio)
    {
      double hz = HZOOM_SCALE * hzoom;
      int index = event->x() / hz;

      emit mouse_time_changed (index / audio->mix_freq * 1000);
    }
  move_marker (event->x());
}

void
SampleView::mouseReleaseEvent (QMouseEvent *event)
{
  move_marker (event->x());
  unsetCursor();

  if (event->button() == Qt::LeftButton)
    button_1_pressed = false;
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
  update();
}
