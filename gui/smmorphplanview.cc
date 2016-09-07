// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmoveindicator.hh"

#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanView::MorphPlanView (MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window) :
  morph_plan (morph_plan),
  morph_plan_window (morph_plan_window),
  need_view_rebuild (true)
{
  vbox = new QVBoxLayout();
  setLayout (vbox);

  connect (morph_plan, SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
  connect (morph_plan, SIGNAL (need_view_rebuild()), this, SLOT (on_need_view_rebuild()));

  on_plan_changed();
}

/* the signals need_view_rebuild() and plan_changed() are always emitted in pairs:

   - in on_need_view_rebuild() we know that we will need to rebuild our ui, so we
     will free the old view widgets

   - then the morph plan can delete operators that are no longer needed, as the
     view widgets that point to them are gone

   - finally in on_plan_changed() we can build new view widgets that point to the new
     operators
*/
void
MorphPlanView::on_need_view_rebuild()
{
  need_view_rebuild = true;

  for (vector<MorphOperatorView *>::const_iterator ovi = m_op_views.begin(); ovi != m_op_views.end(); ovi++)
    delete *ovi;
  m_op_views.clear();
}

void
MorphPlanView::on_plan_changed()
{
  if (!need_view_rebuild)
    return; // nothing to do, view widgets should be fine

  need_view_rebuild = false;

  for (vector<MoveIndicator *>::const_iterator mi = move_indicators.begin(); mi != move_indicators.end(); mi++)
    delete *mi;
  move_indicators.clear();

  // first move indicator
  {
    MoveIndicator *indicator = new MoveIndicator();
    vbox->addWidget (indicator);
    move_indicators.push_back (indicator);
  }

  const vector<MorphOperator *>& operators = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = operators.begin(); oi != operators.end(); oi++)
    {
      MorphOperatorView *op_view = MorphOperatorView::create (*oi, morph_plan_window);
      vbox->addWidget (op_view);
      m_op_views.push_back (op_view);

      connect (op_view, SIGNAL (move_indication (MorphOperator *)), this, SLOT (on_move_indication (MorphOperator *)));

      MoveIndicator *indicator = new MoveIndicator();
      vbox->addWidget (indicator);

      move_indicators.push_back (indicator);
    }

  // control widgets
  for (vector<QWidget *>::const_iterator ci = control_widgets.begin(); ci != control_widgets.end(); ci++)
    {
      vbox->removeWidget (*ci);
      vbox->addWidget (*ci);
    }
}

const vector<MorphOperatorView *>&
MorphPlanView::op_views()
{
  return m_op_views;
}

void
MorphPlanView::on_move_indication (MorphOperator *op)
{
  g_return_if_fail (m_op_views.size() + 1 == move_indicators.size());
  g_return_if_fail (!move_indicators.empty());

  size_t active_i = move_indicators.size() - 1;  // op == NULL -> last move indicator
  if (op)
    {
      for (size_t i = 0; i < m_op_views.size(); i++)
        {
          if (m_op_views[i]->op() == op)
            active_i = i;
        }
    }

  for (size_t i = 0; i < move_indicators.size(); i++)
    move_indicators[i]->set_active (i == active_i);
}

void
MorphPlanView::add_control_widget (QWidget *widget)
{
  control_widgets.push_back (widget);

  on_need_view_rebuild();
  on_plan_changed();
}
