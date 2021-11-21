// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_SAMPLE_EDIT_HH
#define SPECTMORPH_SAMPLE_EDIT_HH

#include "smsimplejackplayer.hh"
#include "smsampleview.hh"
#include "smzoomcontroller.hh"

#include <QWidget>
#include <QApplication>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMainWindow>

#include <memory>

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

class MainWidget : public QWidget
{
  Q_OBJECT

  Audio               audio;
  Wave               *current_wave;

  SimpleJackPlayer   *jack_player;

  SampleView         *sample_view;
  QScrollArea        *scroll_area;
  ZoomController     *zoom_controller;
  QLabel             *time_label;
  QComboBox          *sample_combobox;
  QSlider            *volume_slider;
  QLabel             *volume_label;
  QPushButton        *edit_clip_start;
  QPushButton        *edit_clip_end;
  std::string         marker_filename;
  std::vector<Wave>   waves;

  std::unique_ptr<WavData> samples;

  static std::vector<float> get_clipped_samples (Wave *wave, WavData *samples);
public:
  MainWidget (bool use_jack);
  ~MainWidget();

  void load (const std::string& filename, const std::string& clip_markers);
  void clip (const std::string& export_pattern);

public slots:
  void on_volume_changed (int new_volume);
  void on_combo_changed();
  void on_zoom_changed();
  void on_edit_marker_changed();
  void on_mouse_time_changed (int time);
  void on_next_sample();

  void on_play_clicked();
  void on_save_clicked();
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

  MainWidget *main_widget;

public:
  MainWindow (bool use_jack);

  void load (const std::string& filename, const std::string& clip_markers);
  void clip (const std::string& export_pattern);
};


}

#endif
