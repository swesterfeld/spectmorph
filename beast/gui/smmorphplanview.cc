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
  g_printerr ("plan changed\n");

  if (morph_plan->structure_version() == old_structure_version)
    return; // nothing to do, view widgets should be fine
  old_structure_version = morph_plan->structure_version();

  g_printerr ("structure changed\n");

  vector<Widget *> old_children = get_children();
  for (vector<Widget *>::iterator ci = old_children.begin(); ci != old_children.end(); ci++)
    {
      Widget *view = *ci;
      remove (*view);
      delete view;
    }

  const vector<MorphOperator *>& operators = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = operators.begin(); oi != operators.end(); oi++)
    {
      MorphOperatorView *op_view = (*oi)->create_view (main_window);
      add (*op_view);
      op_view->show();
    }
}
