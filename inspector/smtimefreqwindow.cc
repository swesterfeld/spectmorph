// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimefreqwindow.hh"

using namespace SpectMorph;

TimeFreqWindow::TimeFreqWindow (Navigator *navigator)
{
  time_freq_win_view = new TimeFreqWinView (navigator);

  setCentralWidget (time_freq_win_view);

  resize (800, 600);
}

TimeFreqView *
TimeFreqWindow::time_freq_view()
{
  return time_freq_win_view->time_freq_view();
}

void
TimeFreqWindow::on_dhandle_changed()
{
}

void
TimeFreqWindow::on_position_changed()
{
}

void
TimeFreqWindow::on_analysis_changed()
{
}

void
TimeFreqWindow::on_frequency_grid_changed()
{
}
