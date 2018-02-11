// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"

using namespace SpectMorph;

MorphPlanView::MorphPlanView (Widget *parent, MorphPlan *morph_plan) :
  Widget (parent),
  morph_plan (morph_plan)
{
  connect (morph_plan->signal_plan_changed, this, &MorphPlanView::on_plan_changed);
  connect (morph_plan->signal_need_view_rebuild, this, &MorphPlanView::on_need_view_rebuild);

  need_view_rebuild = true;
  on_plan_changed();
}

void
MorphPlanView::on_plan_changed()
{
  if (!need_view_rebuild)
    return; // nothing to do, view widgets should be fine

  need_view_rebuild = false;

  FixedGrid grid;

  double y = 0;
  for (auto op : morph_plan->operators())
    {
      // MorphOperatorView *op_view = MorphOperatorView::create (*oi, morph_plan_window);
      MorphOperatorView *op_view = new MorphOperatorView (this, op);

      grid.add_widget (op_view, 0, y, 43, 7);
      y += 8;

      m_op_views.push_back (op_view);
    }
}

void
MorphPlanView::on_need_view_rebuild()
{
  need_view_rebuild = true;

  for (auto view : m_op_views)
    delete view;
  m_op_views.clear();
}
