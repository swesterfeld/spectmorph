// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
SampleWindow::on_wav_data_changed()
{
  sample_win_view->load (navigator->get_wav_data(), navigator->get_audio());
}

SampleView *
SampleWindow::sample_view()
{
  return sample_win_view->sample_view();
}

void
SampleWindow::on_next_sample()
{
  Q_EMIT next_sample();
}
