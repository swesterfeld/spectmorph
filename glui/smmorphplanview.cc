// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmorphoutputview.hh"
#include "smmorphlinearview.hh"
#include "smmorphlfoview.hh"

using namespace SpectMorph;

using std::string;

MorphPlanView::MorphPlanView (Widget *parent, MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window) :
  ScrollView (parent),
  morph_plan (morph_plan),
  morph_plan_window (morph_plan_window)
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
      MorphOperatorView *op_view = nullptr;

      string type = op->type();
      if (type == "SpectMorph::MorphSource")
        {
          op_view = new MorphSourceView (this, static_cast<MorphSource *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphOutput")
        {
          op_view = new MorphOutputView (this, static_cast<MorphOutput *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLinear")
        {
          op_view = new MorphLinearView (this, static_cast<MorphLinear *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLFO")
        {
          op_view = new MorphLFOView (this, static_cast<MorphLFO *> (op), morph_plan_window);
        }
      else
        {
          op_view = new MorphOperatorView (this, op, morph_plan_window);
        }

      grid.add_widget (op_view, 0, y, 43, op_view->view_height());
      y += op_view->view_height() + 1;

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
