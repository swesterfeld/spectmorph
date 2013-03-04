// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnavigatorwindow.hh"

#include <QAction>
#include <QMenu>
#include <QMenuBar>

using namespace SpectMorph;

using std::vector;
using std::string;

NavigatorWindow::NavigatorWindow (const string& filename)
{
  navigator = new Navigator (filename);

  /* actions ... */

  QAction *view_time_freq_action = new QAction ("Time/Frequency View", this);

  QAction *view_sample_action = new QAction ("Sample View", this);
  connect (view_sample_action, SIGNAL (triggered()), navigator, SLOT (on_view_sample()));

  QAction *view_spectrum_action = new QAction ("Spectrum View", this);

  QAction *view_lpc_action = new QAction ("LPC View", this);

  QAction *view_fft_params_action = new QAction ("FFT Params", this);

  QAction *view_display_params_action = new QAction ("Display Params", this);

  QAction *view_player_action = new QAction ("Player", this);
  connect (view_player_action, SIGNAL (triggered()), navigator, SLOT (on_view_player()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *view_menu = menu_bar->addMenu ("&View");
  view_menu->addAction (view_time_freq_action);
  view_menu->addAction (view_sample_action);
  view_menu->addAction (view_spectrum_action);
  view_menu->addAction (view_lpc_action);
  view_menu->addAction (view_fft_params_action);
  view_menu->addAction (view_display_params_action);
  view_menu->addAction (view_player_action);

  setCentralWidget (navigator);
  resize (300, 600);
}


