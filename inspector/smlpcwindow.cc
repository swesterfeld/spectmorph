// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlpcwindow.hh"

#include <QVBoxLayout>

using namespace SpectMorph;

LPCWindow::LPCWindow()
{
  resize (600, 600);
  setWindowTitle ("LPC View");

  lpc_view = new LPCView();
  zoom_controller = new ZoomController (this);
  scroll_area = new QScrollArea();
  scroll_area->setWidget (lpc_view);
  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  zoom_controller->set_vscrollbar (scroll_area->verticalScrollBar());

  QGridLayout *grid = new QGridLayout();
  grid->addWidget (scroll_area, 0, 0, 1, 3);
  for (int i = 0; i < 3; i++)
    {
      grid->addWidget (zoom_controller->hwidget (i), 1, i);
      grid->addWidget (zoom_controller->vwidget (i), 2, i);
    }
  setLayout (grid);

  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));
}

void
LPCWindow::set_lpc_model (TimeFreqView *time_freq_view)
{
  lpc_view->set_lpc_model (time_freq_view);
}

void
LPCWindow::on_zoom_changed()
{
  lpc_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}
