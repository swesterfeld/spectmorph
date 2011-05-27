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
public:
  enum ControlType {
    CONTROL_GUI      = 1,
    CONTROL_SIGNAL_1 = 2,
    CONTROL_SIGNAL_2 = 3
  };
protected:
  std::string    load_left, load_right;

  MorphOperator *m_left_op;
  MorphOperator *m_right_op;
  double         m_morphing;
  ControlType    m_control_type;
  bool           m_db_linear;
  bool           m_use_lpc;

public:
  MorphLinear (MorphPlan *morph_plan);
  ~MorphLinear();

  // inherited from MorphOperator
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

  ControlType control_type();
  void set_control_type (ControlType new_control_type);

  bool db_linear();
  void set_db_linear (bool new_db_linear);

  bool use_lpc();
  void set_use_lpc (bool new_use_lpc);

  void on_operator_removed (MorphOperator *op);
};

}

#endif
