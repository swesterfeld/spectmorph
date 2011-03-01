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

#include <gtkmm.h>

#include "smmorphplan.hh"

namespace SpectMorph
{

class MorphPlanView : public Gtk::VBox
{
  MorphPlan *morph_plan;

public:
  MorphPlanView (MorphPlan *morph_plan);

  void on_plan_changed();
};

}

#endif
