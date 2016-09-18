// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_VIEW_HH
#define SPECTMORPH_MORPH_PLAN_VIEW_HH

#include <QWidget>
#include <QVBoxLayout>

#include "smmorphplan.hh"
#include "smmoveindicator.hh"

namespace SpectMorph
{

class MorphPlanWindow;
class MorphPlanView : public QWidget
{
  Q_OBJECT

  MorphPlan                    *morph_plan;
  MorphPlanWindow              *morph_plan_window;

  QVBoxLayout                  *vbox;

  std::vector<MorphOperatorView *> m_op_views;
  std::vector<MoveIndicator *>  move_indicators;
  std::vector<QWidget *>        control_widgets;

  bool                          need_view_rebuild;

public:
  MorphPlanView (MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window);

  const std::vector<MorphOperatorView *>& op_views();
  void add_control_widget (QWidget *widget);

signals:
  void view_widgets_changed();

public slots:
  void on_plan_changed();
  void on_move_indication (MorphOperator *op);
  void on_need_view_rebuild();
};

}

#endif
