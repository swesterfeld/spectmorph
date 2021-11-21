// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smnavigatorwindow.hh"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QCloseEvent>

using namespace SpectMorph;

using std::vector;
using std::string;

NavigatorWindow::NavigatorWindow (const string& filename)
{
  navigator = new Navigator (filename);
  connect (navigator, SIGNAL (title_changed()), this, SLOT (update_title()));

  /* actions ... */

  QAction *view_time_freq_action = new QAction ("Time/Frequency View", this);
  connect (view_time_freq_action, SIGNAL (triggered()), navigator, SLOT (on_view_time_freq()));

  QAction *view_sample_action = new QAction ("Sample View", this);
  connect (view_sample_action, SIGNAL (triggered()), navigator, SLOT (on_view_sample()));

  QAction *view_spectrum_action = new QAction ("Spectrum View", this);
  connect (view_spectrum_action, SIGNAL (triggered()), navigator, SLOT (on_view_spectrum()));

  QAction *view_fft_params_action = new QAction ("FFT Params", this);
  connect (view_fft_params_action, SIGNAL (triggered()), navigator, SLOT (on_view_fft_params()));

  QAction *view_player_action = new QAction ("Player", this);
  connect (view_player_action, SIGNAL (triggered()), navigator, SLOT (on_view_player()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *view_menu = menu_bar->addMenu ("&View");
  view_menu->addAction (view_time_freq_action);
  view_menu->addAction (view_sample_action);
  view_menu->addAction (view_spectrum_action);
  view_menu->addAction (view_fft_params_action);
  view_menu->addAction (view_player_action);

  setCentralWidget (navigator);
  resize (300, 600);

  update_title();
}

void
NavigatorWindow::closeEvent (QCloseEvent *event)
{
  if (navigator->handle_close_event())
    {
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void
NavigatorWindow::update_title()
{
  setWindowTitle (navigator->title().c_str());
}
