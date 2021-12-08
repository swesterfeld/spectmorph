// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smspectrumwindow.hh"
#include "smnavigator.hh"

#include <QGridLayout>

using namespace SpectMorph;

SpectrumWindow::SpectrumWindow (Navigator *navigator)
{
  resize (800, 600);
  setWindowTitle ("Spectrum View");

  spectrum_view = new SpectrumView (navigator);
  zoom_controller = new ZoomController (this, 5000, 5000);

  QGridLayout *grid = new QGridLayout();
  scroll_area = new QScrollArea();
  scroll_area->setWidget (spectrum_view);
  grid->addWidget (scroll_area, 0, 0, 1, 3);
  for (int i = 0; i < 3; i++)
    {
      grid->addWidget (zoom_controller->hwidget (i), 1, i);
      grid->addWidget (zoom_controller->vwidget (i), 2, i);
    }
  setLayout (grid);

  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  zoom_controller->set_vscrollbar (scroll_area->verticalScrollBar());
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));
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
