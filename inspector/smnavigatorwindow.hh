// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_NAVIGATOR_WINDOW_HH
#define SPECTMORPH_NAVIGATOR_WINDOW_HH

#include <string>

#include <QMainWindow>

#include "smnavigator.hh"

namespace SpectMorph {

class NavigatorWindow : public QMainWindow
{
  Q_OBJECT

  Navigator *navigator;
public:
  NavigatorWindow (const std::string& filename);

  void closeEvent (QCloseEvent *event);
public slots:
  void update_title();
};

}

#endif
