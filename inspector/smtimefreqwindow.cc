// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimefreqwindow.hh"

#include <QAction>
#include <QMenuBar>

using namespace SpectMorph;

TimeFreqWindow::TimeFreqWindow (Navigator *navigator)
{
  setWindowTitle ("Time/Frequency View");

  time_freq_win_view = new TimeFreqWinView (navigator);

  /* actions ... */
  QAction *export_action = new QAction ("Export Analysis Data", this);
  connect (export_action, SIGNAL (triggered()), time_freq_view(), SLOT (on_export()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *file_menu = menu_bar->addMenu ("&File");
  file_menu->addAction (export_action);

  setCentralWidget (time_freq_win_view);

  resize (800, 600);
}

TimeFreqView *
TimeFreqWindow::time_freq_view()
{
  return time_freq_win_view->time_freq_view();
}
