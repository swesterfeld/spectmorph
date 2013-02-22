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
