// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimefreqwindow.hh"
#include "smmath.hh"
#include "smnavigator.hh"

#include <QVBoxLayout>

using std::vector;
using std::string;

using namespace SpectMorph;

TimeFreqWindow::TimeFreqWindow (Navigator *navigator) :
  navigator (navigator)
{
  setWindowTitle ("Time/Frequency View");

  m_time_freq_view = new TimeFreqView();
  scroll_area = new QScrollArea();
  scroll_area->setWidgetResizable (true);
  scroll_area->setWidget (m_time_freq_view);

  zoom_controller = new ZoomController (5000, 10000);
  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  zoom_controller->set_vscrollbar (scroll_area->verticalScrollBar());
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));

  QHBoxLayout *position_hbox = new QHBoxLayout();
  position_slider = new QSlider (Qt::Horizontal);
  position_label = new QLabel ("frame 0");
  position_slider->setRange (0, 1000 * 1000);
  position_hbox->addWidget (position_slider);
  position_hbox->addWidget (position_label);
  connect (position_slider, SIGNAL (valueChanged (int)), this, SLOT (on_position_changed()));

  QHBoxLayout *min_db_hbox = new QHBoxLayout();
  min_db_slider = new QSlider (Qt::Horizontal);
  min_db_slider->setRange (-192000, -3000);
  min_db_slider->setValue (-96000);
  min_db_label = new QLabel();
  min_db_hbox->addWidget (min_db_slider);
  min_db_hbox->addWidget (min_db_label);
  connect (min_db_slider, SIGNAL (valueChanged (int)), this, SLOT (on_display_params_changed()));

  QHBoxLayout *boost_hbox = new QHBoxLayout();
  boost_slider = new QSlider (Qt::Horizontal);
  boost_slider->setRange (0, 100000);
  boost_slider->setValue (0);
  boost_label = new QLabel();
  boost_hbox->addWidget (boost_slider);
  boost_hbox->addWidget (boost_label);
  connect (boost_slider, SIGNAL (valueChanged (int)), this, SLOT (on_display_params_changed()));

  on_display_params_changed();

  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget (scroll_area);
  vbox->addWidget (zoom_controller);
  vbox->addLayout (position_hbox);
  vbox->addLayout (min_db_hbox);
  vbox->addLayout (boost_hbox);
  setLayout (vbox);

  connect (navigator->fft_param_window(), SIGNAL (params_changed()), this, SLOT (on_dhandle_changed()));
  connect (m_time_freq_view, SIGNAL (progress_changed()), this, SLOT (on_progress_changed()));

  resize (800, 600);
}

void
TimeFreqWindow::on_dhandle_changed()
{
  m_time_freq_view->load (navigator->get_dhandle(), "fn", navigator->get_audio(),
                          navigator->fft_param_window()->get_analysis_params());
}

void
TimeFreqWindow::on_zoom_changed()
{
  m_time_freq_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}

void
TimeFreqWindow::on_position_changed()
{
  int new_pos = position_slider->value();
  int frames = m_time_freq_view->get_frames();
  int position = CLAMP (sm_round_positive (new_pos / 1000.0 / 1000.0 * frames), 0, frames - 1);
  char buffer[1024];
  sprintf (buffer, "frame %d", position);
  position_label->setText (buffer);
  if (navigator->get_show_position())
    m_time_freq_view->set_position (position);
  else
    m_time_freq_view->set_position (-1);
}

void
TimeFreqWindow::on_analysis_changed()
{
  m_time_freq_view->set_show_analysis (navigator->get_show_analysis());
}

void
TimeFreqWindow::on_frequency_grid_changed()
{
  m_time_freq_view->set_show_frequency_grid (navigator->get_show_frequency_grid());
}

void
TimeFreqWindow::on_progress_changed()
{
  navigator->fft_param_window()->set_progress (m_time_freq_view->get_progress());
}

void
TimeFreqWindow::on_display_params_changed()
{
  double min_db = min_db_slider->value() / 1000.0;
  double boost = boost_slider->value() / 1000.0;

  min_db_label->setText (Birnet::string_printf ("min_db %.2f", min_db).c_str());
  boost_label->setText (Birnet::string_printf ("boost %.2f", boost).c_str());
  m_time_freq_view->set_display_params (min_db, boost);
}

TimeFreqView*
TimeFreqWindow::time_freq_view()
{
  return m_time_freq_view;
}
