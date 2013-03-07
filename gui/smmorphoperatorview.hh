// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smmorphplanwindow.hh"

#include <QGroupBox>

namespace SpectMorph
{

class MorphOperator;
class MorphOperatorView : public QGroupBox
{
  Q_OBJECT
protected:
  QMenu            *context_menu;
  MorphOperator    *m_op;
  MorphPlanWindow  *morph_plan_window;
  bool              remove;
  bool              in_move;

  void contextMenuEvent (QContextMenuEvent *event);
  void mousePressEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void mouseReleaseEvent (QMouseEvent *event);

public:
  MorphOperatorView (MorphOperator *op, MorphPlanWindow *morph_plan_window);

  MorphOperator *op();

  static MorphOperatorView *create (MorphOperator *op, MorphPlanWindow *window);

public slots:
  void on_operators_changed();
  void on_rename();
  void on_remove();

signals:
  void move_indication (MorphOperator *op);
};

}

#endif
