// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmorphoutputview.hh"
#include "smmorphlinearview.hh"
#include "smmorphlfoview.hh"
#include "smmorphgridview.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanView::MorphPlanView (Widget *parent, Widget *output_parent, MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window) :
  Widget (parent),
  output_parent (output_parent),
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
          op_view = new MorphOutputView (output_parent, static_cast<MorphOutput *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLinear")
        {
          op_view = new MorphLinearView (this, static_cast<MorphLinear *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLFO")
        {
          op_view = new MorphLFOView (this, static_cast<MorphLFO *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphGrid")
        {
          op_view = new MorphGridView (this, static_cast<MorphGrid *> (op), morph_plan_window);
        }
      else
        {
          op_view = new MorphOperatorView (this, op, morph_plan_window);
        }
      connect (op_view->signal_fold_changed, this, &MorphPlanView::update_positions);
      connect (op_view->signal_move_indication, this, &MorphPlanView::on_move_indication);

      m_op_views.push_back (op_view);
    }
  update_positions();
}

void
MorphPlanView::update_positions()
{
  FixedGrid grid;

  double y = 0;

  for (auto op_view : m_op_views)
    {
      double view_height;
      if (op_view->op()->folded())
        {
          view_height = 4;
        }
      else
        {
          view_height = op_view->view_height();
        }

      if (op_view->is_output())
        {
          grid.add_widget (op_view, 0, 0, 43, view_height);
          grid.add_widget (op_view->body_widget, 2, 4, 40, view_height - 5);
        }
      else
        {
          grid.add_widget (op_view, 0, y, 43, view_height);
          grid.add_widget (op_view->body_widget, 2, 4, 40, view_height - 5);
          y += view_height + 1;
        }
    }

  height = (y - 1) * 8;
  width  = 43 * 8;
  signal_widget_size_changed();
}

void
MorphPlanView::on_need_view_rebuild()
{
  need_view_rebuild = true;

  for (auto view : m_op_views)
    delete view;
  m_op_views.clear();
  move_ind_widget.reset();
}

const vector<MorphOperatorView *>&
MorphPlanView::op_views()
{
  return m_op_views;
}

void
MorphPlanView::on_move_indication (MorphOperator *op)
{
  MorphOperatorView *view = nullptr;

  for (auto v : m_op_views)
    {
      if (op) // search for matching view
        {
          if (v->op() == op)
            view = v;
        }
      else if (!v->is_output())  // last view for op == null
        view = v;
    }
  if (!view) // shouldn't happen
    return;

  move_ind_widget.reset (new Widget (this));

  move_ind_widget->set_background_color (ThemeColor::MENU_ITEM);
  move_ind_widget->x = view->x;
  move_ind_widget->width = view->width;
  move_ind_widget->height = 4;

  if (op) // before view
    {
      move_ind_widget->y = view->y - 6;
    }
  else // after view
    {
      move_ind_widget->y = view->y + view->height + 2;
    }
}
