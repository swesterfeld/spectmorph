// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include <QWidget>

namespace SpectMorph
{

class MorphGridWidget : public QWidget
{
public:
  MorphGridWidget();

  void paintEvent (QPaintEvent *event);
};

}

#endif

