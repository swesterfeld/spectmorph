/*
 * Copyright (C) 2010 Stefan Westerfeld
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
