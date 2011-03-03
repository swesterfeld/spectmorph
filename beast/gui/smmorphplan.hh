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

#ifndef SPECTMORPH_MORPH_PLAN_HH
#define SPECTMORPH_MORPH_PLAN_HH

#include "smindex.hh"
#include "smmorphoperator.hh"
#include <sigc++/sigc++.h>

namespace SpectMorph
{

class MorphPlan
{
  Index                        m_index;
  std::vector<MorphOperator *> m_operators;
  bool in_restore;

public:
  MorphPlan();
  ~MorphPlan();

  bool   load_index (const std::string& filename);
  Index *index();

  void add_operator (MorphOperator *op);
  const std::vector<MorphOperator *>& operators();

  sigc::signal<void> signal_plan_changed;
  sigc::signal<void> signal_index_changed;

  void set_plan_str (const std::string& plan_str);
};

}

#endif
