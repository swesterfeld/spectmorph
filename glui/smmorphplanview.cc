// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmorphwavsourceview.hh"
#include "smmorphoutputview.hh"
#include "smmorphlinearview.hh"
#include "smmorphlfoview.hh"
#include "smmorphgridview.hh"
#include "smoperatorrolemap.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanView::MorphPlanView (Widget *plan_parent, Widget *output_parent, MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window) :
  plan_parent (plan_parent),
  output_parent (output_parent),
  morph_plan (morph_plan),
  morph_plan_window (morph_plan_window)
{
  control_widget = new MorphPlanControl (output_parent, morph_plan);

  connect (morph_plan->signal_plan_changed, this, &MorphPlanView::on_plan_changed);
  connect (morph_plan->signal_need_view_rebuild, this, &MorphPlanView::on_need_view_rebuild);

  need_view_rebuild = true;
  on_plan_changed();
}

void
MorphPlanView::on_plan_changed()
{
  if (!need_view_rebuild)
    {
      update_roles(); /* roles could have changed */
      return; // nothing to do, view widgets should be fine
    }

  need_view_rebuild = false;

  for (auto op : morph_plan->operators())
    {
      // MorphOperatorView *op_view = MorphOperatorView::create (*oi, morph_plan_window);
      MorphOperatorView *op_view = nullptr;

      string type = op->type();
      if (type == "SpectMorph::MorphSource")
        {
          op_view = new MorphSourceView (plan_parent, static_cast<MorphSource *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphWavSource")
        {
          op_view = new MorphWavSourceView (plan_parent, static_cast<MorphWavSource *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphOutput")
        {
          op_view = new MorphOutputView (output_parent, static_cast<MorphOutput *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLinear")
        {
          op_view = new MorphLinearView (plan_parent, static_cast<MorphLinear *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphLFO")
        {
          op_view = new MorphLFOView (plan_parent, static_cast<MorphLFO *> (op), morph_plan_window);
        }
      else if (type == "SpectMorph::MorphGrid")
        {
          op_view = new MorphGridView (plan_parent, static_cast<MorphGrid *> (op), morph_plan_window);
        }
      else
        {
          op_view = new MorphOperatorView (plan_parent, op, morph_plan_window);
        }
      connect (op_view->signal_size_changed, this, &MorphPlanView::update_positions);
      connect (op_view->signal_move_indication, this, &MorphPlanView::on_move_indication);

      m_op_views.push_back (op_view);
    }

  update_roles();
  update_positions();
}

void
MorphPlanView::update_positions()
{
  FixedGrid grid;

  double y = 0;
  double output_height = 0;

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
          output_height += view_height + 1;
        }
      else
        {
          grid.add_widget (op_view, 0, y, 43, view_height);
          grid.add_widget (op_view->body_widget, 2, 4, 40, view_height - 5);
          y += view_height + 1;
        }
    }

  plan_parent->set_height ((y - 1) * 8);
  plan_parent->set_width (43 * 8);

  if (output_height < 53)
    output_height = 53;
  double cw_height = control_widget->view_height();
  grid.add_widget (control_widget, 0, output_height, 43, cw_height);
  output_height += cw_height;
  output_parent->set_height (output_height * 8);
  output_parent->set_width (43 * 8);

  signal_widget_size_changed();

  /*
   * it would be possible to avoid a full update if moving widgets on the screen
   * would automatically update old/new region occupied by the widget FIXME: FILTER
   *
   * however update_positions() doesn't happen often, so it should be cheap enough
   */
  //update_full();
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
MorphPlanView::update_roles()
{
  op_role_map.rebuild (morph_plan);

  for (auto op_view : m_op_views)
    {
      int role = op_role_map.get (op_view->op());
      op_view->set_role (role);
    }
}

void
MorphPlanView::on_move_indication (MorphOperator *op, bool done)
{
  if (done)
    {
      move_ind_widget.reset();
      return;
    }
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

  move_ind_widget.reset (new Widget (plan_parent));

  move_ind_widget->set_background_color (ThemeColor::MENU_ITEM);
  move_ind_widget->set_x (view->x());
  move_ind_widget->set_width (view->width());
  move_ind_widget->set_height (4);

  if (op) // before view
    {
      move_ind_widget->set_y (view->y() - 6);
    }
  else // after view
    {
      move_ind_widget->set_y (view->y() + view->height() + 2);
    }
}
