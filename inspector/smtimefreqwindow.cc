// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimefreqwindow.hh"
#include "smmath.hh"
#include "smnavigator.hh"
#include "smutils.hh"

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

  zoom_controller = new ZoomController (this, 5000, 10000);
  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  zoom_controller->set_vscrollbar (scroll_area->verticalScrollBar());
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));

  position_slider = new QSlider (Qt::Horizontal);
  position_label = new QLabel ("0");
  position_slider->setRange (0, 1000 * 1000);
  connect (position_slider, SIGNAL (valueChanged (int)), this, SLOT (on_position_changed()));

  min_db_slider = new QSlider (Qt::Horizontal);
  min_db_slider->setRange (-192000, -3000);
  min_db_slider->setValue (-96000);
  min_db_label = new QLabel();
  connect (min_db_slider, SIGNAL (valueChanged (int)), this, SLOT (on_display_params_changed()));

  boost_slider = new QSlider (Qt::Horizontal);
  boost_slider->setRange (0, 100000);
  boost_slider->setValue (0);
  boost_label = new QLabel();
  connect (boost_slider, SIGNAL (valueChanged (int)), this, SLOT (on_display_params_changed()));

  on_display_params_changed();

  QGridLayout *grid = new QGridLayout();
  grid->addWidget (scroll_area, 0, 0, 1, 3);
  for (int i = 0; i < 3; i++)
    {
      grid->addWidget (zoom_controller->hwidget (i), 1, i);
      grid->addWidget (zoom_controller->vwidget (i), 2, i);
    }
  grid->addWidget (new QLabel ("Position"), 3, 0);
  grid->addWidget (position_slider, 3, 1);
  grid->addWidget (position_label, 3, 2);

  grid->addWidget (new QLabel ("Min dB"), 4, 0);
  grid->addWidget (min_db_slider, 4, 1);
  grid->addWidget (min_db_label, 4, 2);

  grid->addWidget (new QLabel ("Boost"), 5, 0);
  grid->addWidget (boost_slider, 5, 1);
  grid->addWidget (boost_label, 5, 2);
  setLayout (grid);

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
  sprintf (buffer, "%d", position);
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

  min_db_label->setText (string_printf ("%.2f", min_db).c_str());
  boost_label->setText (string_printf ("%.2f", boost).c_str());
  m_time_freq_view->set_display_params (min_db, boost);
}

TimeFreqView*
TimeFreqWindow::time_freq_view()
{
  return m_time_freq_view;
}
