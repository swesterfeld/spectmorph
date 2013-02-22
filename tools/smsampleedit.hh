/*
 * Copyright (C) 2010-2013 Stefan Westerfeld
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

#ifndef SPECTMORPH_SAMPLE_EDIT_HH
#define SPECTMORPH_SAMPLE_EDIT_HH

#include "smsimplejackplayer.hh"
#include "smsampleview.hh"
#include "smwavloader.hh"

#include <QWidget>
#include <QApplication>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

namespace SpectMorph
{

class SampleEditMarkers : public SampleView::Markers
{
public:
  std::vector<float> positions;
  std::vector<bool>  m_valid;

  SampleEditMarkers() :
    positions (2),
    m_valid (positions.size())
  {
  }
  size_t
  count()
  {
    return positions.size();
  }
  bool
  valid (size_t m)
  {
    return m_valid[m];
  }
  float
  position (size_t m)
  {
    g_return_val_if_fail (m >= 0 && m < positions.size(), 0.0);

    return positions[m];
  }
  void
  set_position (size_t m, float new_pos)
  {
    g_return_if_fail (m >= 0 && m < positions.size());

    positions[m] = new_pos;
    m_valid[m] = true;
  }
  SampleView::EditMarkerType
  type (size_t m)
  {
    if (m == 0)
      return SampleView::MARKER_CLIP_START;
    else if (m == 1)
      return SampleView::MARKER_CLIP_END;
    else
      return SampleView::MARKER_NONE;
  }
  void
  clear (size_t m)
  {
    g_return_if_fail (m >= 0 && m < positions.size());

    m_valid[m] = false;
  }
  float
  clip_start (bool& v)
  {
    v = m_valid[0];
    return positions[0];
  }
  float
  clip_end (bool& v)
  {
    v = m_valid[1];
    return positions[1];
  }
  void
  set_clip_start (float pos)
  {
    m_valid[0] = true;
    positions[0] = pos;
  }
  void
  set_clip_end (float pos)
  {
    m_valid[1] = true;
    positions[1] = pos;
  }
};

struct Wave
{
  std::string         path;
  std::string         label;
  int                 midi_note;

  SampleEditMarkers   markers;
};

class MainWindow : public QWidget
{
  Q_OBJECT
#if 0
  Gtk::HBox           button_hbox;
  Gtk::VBox           vbox;

  Gtk::HBox           volume_hbox;
  Gtk::Label          volume_label;

  ZoomController      zoom_controller;
  bool                in_update_buttons;
#endif
  Audio               audio;
  Wave               *current_wave;

  SimpleJackPlayer    jack_player;

  bool                in_update_buttons;
  SampleView         *sample_view;
  QScrollArea        *scroll_area;
  QLabel             *time_label;
  QComboBox          *sample_combobox;
  QSlider            *volume_slider;
  QLabel             *volume_label;
  QPushButton        *edit_clip_start;
  QPushButton        *edit_clip_end;
  std::string         marker_filename;
  WavLoader          *samples;
  std::vector<Wave>   waves;

  std::vector<float> get_clipped_samples (Wave *wave, WavLoader *samples);
public:
  MainWindow();
  ~MainWindow();

#if 0
  void on_edit_marker_changed (SampleView::EditMarkerType marker_type);
  void on_play_clicked();
  void on_save_clicked();
  void on_zoom_changed();
  void on_next_sample();
  void on_combo_changed();
  void on_mouse_time_changed (int time);
#endif

  void load (const std::string& filename, const std::string& clip_markers);
  void clip (const std::string& export_pattern);

#if 0
  void on_resized (int old_width, int new_width);
#endif
public slots:
  void on_volume_changed (int new_volume);
  void on_combo_changed();
};

}

#endif
