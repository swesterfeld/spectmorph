// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MOVE_INDICATOR_HH
#define SPECTMORPH_MOVE_INDICATOR_HH

#include <QWidget>

namespace SpectMorph
{

class MoveIndicator : public QWidget
{
protected:
  bool m_active;

public:
  MoveIndicator();

  void paintEvent (QPaintEvent *event);
  void set_active (bool active);
};

}

#endif
