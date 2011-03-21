/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphplanview.hh"
#include "smmorphsourceview.hh"
#include "smmoveindicator.hh"
#include <birnet/birnet.hh>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanView::MorphPlanView (MorphPlan *morph_plan, MainWindow *main_window) :
  morph_plan (morph_plan),
  main_window (main_window)
{
  morph_plan->signal_plan_changed.connect (sigc::mem_fun (*this, &MorphPlanView::on_plan_changed));

  old_structure_version = morph_plan->structure_version() - 1;
}

void
MorphPlanView::on_plan_changed()
{
  if (morph_plan->structure_version() == old_structure_version)
    return; // nothing to do, view widgets should be fine
  old_structure_version = morph_plan->structure_version();

  // delete old MorphOperatorView and MoveIndicator widgets
  vector<Widget *> old_children = get_children();
  for (vector<Widget *>::iterator ci = old_children.begin(); ci != old_children.end(); ci++)
    {
      Widget *view = *ci;
      remove (*view);
      delete view;
    }

  m_op_views.clear();
  move_indicators.clear();

  /*** rebuild gui elements ***/

  // first move indicator
  {
    MoveIndicator *indicator = new MoveIndicator();
    pack_start (*indicator, Gtk::PACK_SHRINK);
    indicator->show();
    move_indicators.push_back (indicator);
  }

  const vector<MorphOperator *>& operators = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = operators.begin(); oi != operators.end(); oi++)
    {
      MorphOperatorView *op_view = MorphOperatorView::create (*oi, main_window);
      pack_start (*op_view, Gtk::PACK_SHRINK);
      op_view->show();
      m_op_views.push_back (op_view);

      op_view->signal_move_indication.connect (sigc::mem_fun (*this, &MorphPlanView::on_move_indication));

      MoveIndicator *indicator = new MoveIndicator();
      pack_start (*indicator, Gtk::PACK_SHRINK);
      indicator->show();

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
