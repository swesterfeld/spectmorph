// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smspectrumwindow.hh"
#include "smnavigator.hh"

#include <QVBoxLayout>

using namespace SpectMorph;

SpectrumWindow::SpectrumWindow (Navigator *navigator)
{
  resize (800, 600);
  setWindowTitle ("Spectrum View");

  spectrum_view = new SpectrumView (navigator);
  zoom_controller = new ZoomController (5000, 5000);

  QVBoxLayout *vbox = new QVBoxLayout();
  scroll_area = new QScrollArea();
  scroll_area->setWidget (spectrum_view);
  vbox->addWidget (scroll_area);
  vbox->addWidget (zoom_controller);

  setLayout (vbox);

  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  zoom_controller->set_vscrollbar (scroll_area->verticalScrollBar());
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));
  connect (navigator->display_param_window(), SIGNAL (params_changed()),
           spectrum_view, SLOT (on_display_params_changed()));
}

void
SpectrumWindow::set_spectrum_model (TimeFreqView *time_freq_view)
{
  spectrum_view->set_spectrum_model (time_freq_view);
}

void
SpectrumWindow::on_zoom_changed()
{
  spectrum_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}
