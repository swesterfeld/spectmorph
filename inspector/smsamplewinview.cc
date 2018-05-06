// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smsamplewinview.hh"
#include "smnavigator.hh"
#include "smutils.hh"

#include <QVBoxLayout>

#define LOOP_NONE_TEXT              "No loop"
#define LOOP_FRAME_FORWARD_TEXT     "Frame loop forward"
#define LOOP_FRAME_PING_PONG_TEXT   "Frame loop ping-pong"
#define LOOP_TIME_FORWARD_TEXT      "Time loop forward"
#define LOOP_TIME_PING_PONG_TEXT    "Time loop ping-pong"

using namespace SpectMorph;

using std::string;

SampleWinView::SampleWinView (Navigator *navigator)
{
  this->navigator = navigator;

  zoom_controller = new ZoomController (this, 1, 5000, 10, 5000);
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));

  m_sample_view = new SampleView();
  time_label = new QLabel();
  connect (m_sample_view, SIGNAL (mouse_time_changed (int)), this, SLOT (on_mouse_time_changed (int)));
  on_mouse_time_changed (0);

  edit_loop_start = new QPushButton ("Edit Loop Start");
  edit_loop_end = new QPushButton ("Edit Loop End");

  connect (edit_loop_start, SIGNAL (clicked()), this, SLOT (on_edit_marker_changed()));
  connect (edit_loop_end, SIGNAL (clicked()), this, SLOT (on_edit_marker_changed()));

  edit_loop_start->setCheckable (true);
  edit_loop_end->setCheckable (true);

  loop_type_combo = new QComboBox();
  loop_type_combo->addItem (LOOP_NONE_TEXT);
  loop_type_combo->addItem (LOOP_FRAME_FORWARD_TEXT);
  loop_type_combo->addItem (LOOP_FRAME_PING_PONG_TEXT);
  loop_type_combo->addItem (LOOP_TIME_FORWARD_TEXT);
  loop_type_combo->addItem (LOOP_TIME_PING_PONG_TEXT);

  connect (loop_type_combo, SIGNAL (currentIndexChanged (int)), this, SLOT (on_loop_type_changed()));

  show_tuning_button = new QPushButton ("Show Tuning");
  connect (show_tuning_button, SIGNAL (clicked()), this, SLOT (on_show_tuning_changed()));
  show_tuning_button->setCheckable (true);

  scroll_area = new QScrollArea();
  scroll_area->setWidgetResizable (true);
  scroll_area->setWidget (m_sample_view);

  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  QGridLayout *grid = new QGridLayout();
  grid->addWidget (scroll_area, 0, 0, 1, 3);
  for (int i = 0; i < 3; i++)
    {
      grid->addWidget (zoom_controller->hwidget (i), 1, i);
      grid->addWidget (zoom_controller->vwidget (i), 2, i);
    }
  QHBoxLayout *button_hbox = new QHBoxLayout();
  button_hbox->addWidget (time_label);
  button_hbox->addWidget (edit_loop_start);
  button_hbox->addWidget (edit_loop_end);
  button_hbox->addWidget (loop_type_combo);
  button_hbox->addWidget (show_tuning_button);

  grid->addLayout (button_hbox, 3, 0, 1, 3);

  setLayout (grid);
}

void
SampleWinView::load (const WavData *wav_data, Audio *audio)
{
  m_sample_view->load (wav_data, audio);
  if (audio)
    {
      if (audio->loop_type == Audio::LOOP_NONE)
        loop_type_combo->setCurrentText (LOOP_NONE_TEXT);
      else if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
        loop_type_combo->setCurrentText (LOOP_FRAME_FORWARD_TEXT);
      else if (audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
        loop_type_combo->setCurrentText (LOOP_FRAME_PING_PONG_TEXT);
      else if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
        loop_type_combo->setCurrentText (LOOP_TIME_FORWARD_TEXT);
      else if (audio->loop_type == Audio::LOOP_TIME_PING_PONG)
        loop_type_combo->setCurrentText (LOOP_TIME_PING_PONG_TEXT);
      else
        {
          g_assert_not_reached();
        }
    }
}

void
SampleWinView::on_zoom_changed()
{
  m_sample_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}

void
SampleWinView::on_mouse_time_changed (int time)
{
  int ms = time % 1000;
  time /= 1000;
  int s = time % 60;
  time /= 60;
  int m = time;
  time_label->setText (string_locale_printf ("Time: %02d:%02d:%03d ms", m, s, ms).c_str());
}

void
SampleWinView::on_edit_marker_changed()
{
  SampleView::EditMarkerType marker_type;

  QPushButton *btn = qobject_cast<QPushButton *> (sender());
  if (btn == edit_loop_start)
    marker_type = SampleView::MARKER_LOOP_START;
  else if (btn == edit_loop_end)
    marker_type = SampleView::MARKER_LOOP_END;
  else
    g_assert_not_reached();

  if (m_sample_view->edit_marker_type() == marker_type)  // we're selected already -> turn it off
    marker_type = SampleView::MARKER_NONE;

  m_sample_view->set_edit_marker_type (marker_type);

  edit_loop_start->setChecked (marker_type == SampleView::MARKER_LOOP_START);
  edit_loop_end->setChecked (marker_type == SampleView::MARKER_LOOP_END);
}

void
SampleWinView::on_show_tuning_changed()
{
  m_sample_view->set_show_tuning (show_tuning_button->isChecked());
}

void
SampleWinView::on_loop_type_changed()
{
  Audio *audio = navigator->get_audio();
  if (audio)
    {
      string text = loop_type_combo->currentText().toLatin1().data();
      Audio::LoopType new_loop_type;

      if (text == LOOP_NONE_TEXT)
        new_loop_type = Audio::LOOP_NONE;
      else if (text == LOOP_FRAME_FORWARD_TEXT)
        new_loop_type = Audio::LOOP_FRAME_FORWARD;
      else if (text == LOOP_FRAME_PING_PONG_TEXT)
        new_loop_type = Audio::LOOP_FRAME_PING_PONG;
      else if (text == LOOP_TIME_FORWARD_TEXT)
        new_loop_type = Audio::LOOP_TIME_FORWARD;
      else if (text == LOOP_TIME_PING_PONG_TEXT)
        new_loop_type = Audio::LOOP_TIME_PING_PONG;
      else
        {
          g_assert_not_reached();
        }

      if (new_loop_type != audio->loop_type)
        {
          Q_EMIT audio_edit();
          audio->loop_type = new_loop_type;
        }
    }
}

SampleView *
SampleWinView::sample_view()
{
  return m_sample_view;
}
