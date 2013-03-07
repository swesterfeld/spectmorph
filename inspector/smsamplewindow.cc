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

#include "smsamplewindow.hh"
#include "smsamplewinview.hh"
#include "smnavigator.hh"

#include <QAction>
#include <QMenuBar>

#include <iostream>

using namespace SpectMorph;

SampleWindow::SampleWindow (Navigator *navigator)
{
  this->navigator = navigator;

  sample_win_view = new SampleWinView (navigator);
  connect (sample_win_view, SIGNAL (audio_edit()), navigator, SLOT (on_audio_edit()));

  /* actions ... */
  QAction *next_action = new QAction ("Next Sample", this);
  next_action->setShortcut (QString ("n"));
  connect (next_action, SIGNAL (triggered()), this, SLOT (on_next_sample()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *sample_menu = menu_bar->addMenu ("&Sample");
  sample_menu->addAction (next_action);

  setCentralWidget (sample_win_view);
  setWindowTitle ("Sample View");
  resize (800, 500);
}

void
SampleWindow::on_dhandle_changed()
{
  sample_win_view->load (navigator->get_dhandle(), navigator->get_audio());
}

SampleView *
SampleWindow::sample_view()
{
  return sample_win_view->sample_view();
}

void
SampleWindow::on_next_sample()
{
  emit next_sample();
}
