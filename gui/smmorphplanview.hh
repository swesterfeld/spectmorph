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

  int                           old_structure_version;
public:
  MorphPlanView (MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window);

  const std::vector<MorphOperatorView *>& op_views();

public slots:
  void on_plan_changed();
  void on_move_indication (MorphOperator *op);
};

}

#endif
