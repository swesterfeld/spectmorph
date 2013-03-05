// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NAVIGATOR_WINDOW_HH
#define SPECTMORPH_NAVIGATOR_WINDOW_HH

#include <string>

#include <QMainWindow>

#include "smnavigator.hh"

namespace SpectMorph {

class NavigatorWindow : public QMainWindow
{
  Navigator *navigator;
public:
  NavigatorWindow (const std::string& filename);

  void closeEvent (QCloseEvent *event);
};

}

#endif
