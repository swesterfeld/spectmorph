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
#include "smobject.hh"
#include "smaudio.hh"

namespace SpectMorph
{

class MorphPlan : public Object
{
  Q_OBJECT
  Index                        m_index;
  std::vector<MorphOperator *> m_operators;
  int                          m_structure_version;

  std::string                  index_filename;
  bool                         in_restore;

public:
  MorphPlan();
  ~MorphPlan();

  bool         load_index (const std::string& filename);
  const Index *index();

  void add_operator (MorphOperator *op, const std::string& name = "", const std::string& id = "");
  const std::vector<MorphOperator *>& operators();
  void remove (MorphOperator *op);
  void move (MorphOperator *op, MorphOperator *op_next);

  void set_plan_str (const std::string& plan_str);
  void emit_plan_changed();
  void emit_index_changed();

  BseErrorType save (GenericOut *file);
  BseErrorType load (GenericIn *in);
  void clear();

  int  structure_version();

  static std::string id_chars();
  static std::string generate_id();

signals:
  void plan_changed();
  void index_changed();
  void operator_removed (MorphOperator *op);
};

typedef RefPtr<MorphPlan> MorphPlanPtr;

}

#endif
