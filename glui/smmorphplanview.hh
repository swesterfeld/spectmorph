// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_VIEW_HH
#define SPECTMORPH_MORPH_PLAN_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smmorphoperatorview.hh"
#include "smscrollview.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanWindow;

class MorphPlanView : public Widget
{
  Widget          *output_parent;
  MorphPlan       *morph_plan;
  MorphPlanWindow *morph_plan_window;
  bool             need_view_rebuild;

  std::vector<MorphOperatorView *> m_op_views;

  std::unique_ptr<Widget> move_ind_widget;
public:
  Signal<> signal_widget_size_changed;

  MorphPlanView (Widget *parent, Widget *output_parent, MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window);

  void update_positions();
  const std::vector<MorphOperatorView *>& op_views();

  void on_plan_changed();
  void on_move_indication (MorphOperator *op, bool done);
  void on_need_view_rebuild();
};

}

#endif
