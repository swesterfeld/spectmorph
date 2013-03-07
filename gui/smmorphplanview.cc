// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmoveindicator.hh"
#include <birnet/birnet.hh>

#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanView::MorphPlanView (MorphPlan *morph_plan, MorphPlanWindow *morph_plan_window) :
  morph_plan (morph_plan),
  morph_plan_window (morph_plan_window)
{
  vbox = new QVBoxLayout();
  setLayout (vbox);

  connect (morph_plan, SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));

  old_structure_version = morph_plan->structure_version() - 1;
  on_plan_changed();
}

void
MorphPlanView::on_plan_changed()
{
  if (morph_plan->structure_version() == old_structure_version)
    return; // nothing to do, view widgets should be fine
  old_structure_version = morph_plan->structure_version();

  for (vector<MorphOperatorView *>::const_iterator ovi = m_op_views.begin(); ovi != m_op_views.end(); ovi++)
    delete *ovi;
  m_op_views.clear();

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
