// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smsamplewinview.hh"

#include <QVBoxLayout>

using namespace SpectMorph;

SampleWinView::SampleWinView()
{
  zoom_controller = new ZoomController (1, 5000, 10, 5000);
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));

  sample_view = new SampleView();

  scroll_area = new QScrollArea();
  scroll_area->setWidgetResizable (true);
  scroll_area->setWidget (sample_view);

  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget (scroll_area);
  vbox->addWidget (zoom_controller);
  setLayout (vbox);
}

void
SampleWinView::load (GslDataHandle *dhandle, Audio *audio)
{
  sample_view->load (dhandle, audio);
  if (audio)
    {
#if 0
      if (audio->loop_type == Audio::LOOP_NONE)
        loop_type_combo.set_active_text (LOOP_NONE_TEXT);
      else if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
        loop_type_combo.set_active_text (LOOP_FRAME_FORWARD_TEXT);
      else if (audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
        loop_type_combo.set_active_text (LOOP_FRAME_PING_PONG_TEXT);
      else if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
        loop_type_combo.set_active_text (LOOP_TIME_FORWARD_TEXT);
      else if (audio->loop_type == Audio::LOOP_TIME_PING_PONG)
        loop_type_combo.set_active_text (LOOP_TIME_PING_PONG_TEXT);
      else
        {
          g_assert_not_reached();
        }
#endif
    }
}

void
SampleWinView::on_zoom_changed()
{
  sample_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}


