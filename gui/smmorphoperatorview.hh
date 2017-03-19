// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smmorphplanwindow.hh"

#include <QGroupBox>
#include <QToolButton>

namespace SpectMorph
{

class MorphOperator;
class MorphOperatorView : public QFrame
{
  Q_OBJECT
protected:
  QLabel           *head_label;
  QWidget          *body_widget;
  QMenu            *context_menu;
  QToolButton      *fold_button;
  MorphOperator    *m_op;
  MorphPlanWindow  *morph_plan_window;
  bool              remove;
  bool              in_move;

  void contextMenuEvent (QContextMenuEvent *event);
  void mousePressEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void mouseReleaseEvent (QMouseEvent *event);

  void set_body_layout (QLayout *layout);
  void update_body_visible();

public:
  MorphOperatorView (MorphOperator *op, MorphPlanWindow *morph_plan_window);

  MorphOperator *op();

  static MorphOperatorView *create (MorphOperator *op, MorphPlanWindow *window);

public slots:
  void on_operators_changed();
  void on_rename();
  void on_remove();
  void on_fold_clicked();

signals:
  void move_indication (MorphOperator *op);
  void need_resize();
};

}

#endif
