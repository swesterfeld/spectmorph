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

#ifndef SPECTMORPH_MORPH_LINEAR_HH
#define SPECTMORPH_MORPH_LINEAR_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphLinear : public MorphOperator
{
protected:
  std::string    load_left, load_right;

  MorphOperator *m_left_op;
  MorphOperator *m_right_op;
  double         m_morphing;

public:
  MorphLinear (MorphPlan *morph_plan);

  // inherited from MorphOperator
  MorphOperatorView *create_view (MainWindow *main_window);
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load();
  OutputType         output_type();

  MorphOperator *left_op();
  void set_left_op (MorphOperator *op);

  MorphOperator *right_op();
  void set_right_op (MorphOperator *op);

  double morphing();
  void set_morphing (double new_morphing);

  void on_operator_removed (MorphOperator *op);
};

}

#endif
