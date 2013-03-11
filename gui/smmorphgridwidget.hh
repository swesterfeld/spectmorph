// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include <QWidget>

#include "smmorphgrid.hh"

namespace SpectMorph
{

class MorphGridWidget : public QWidget
{
  Q_OBJECT

  std::vector<int> x_coord;
  std::vector<int> y_coord;

  int selected_x;
  int selected_y;

  MorphGrid *morph_grid;

  void mousePressEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void mouseReleaseEvent (QMouseEvent *event);

public:
  MorphGridWidget (MorphGrid *morph_grid);

  void paintEvent (QPaintEvent *event);
  bool has_selection();

public slots:
  void on_plan_changed();

signals:
  void selection_changed();
};

}

#endif

