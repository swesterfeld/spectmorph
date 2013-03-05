/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include "smlpcwindow.hh"

#include <QVBoxLayout>

using namespace SpectMorph;

LPCWindow::LPCWindow()
{
  resize (600, 600);
  setWindowTitle ("LPC View");

  lpc_view = new LPCView();
  zoom_controller = new ZoomController();
  scroll_area = new QScrollArea();
  scroll_area->setWidget (lpc_view);

  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget (scroll_area);
  vbox->addWidget (zoom_controller);
  setLayout (vbox);
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
