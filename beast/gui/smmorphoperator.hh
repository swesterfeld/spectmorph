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

#ifndef SPECTMORPH_MORPH_OPERATOR_HH
#define SPECTMORPH_MORPH_OPERATOR_HH

#include "smoutfile.hh"
#include "sminfile.hh"

namespace SpectMorph
{

class MorphOperatorView;
class MorphPlan;

class MorphOperator
{
protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;

public:
  enum OutputType {
    OUTPUT_NONE,
    OUTPUT_AUDIO
  };
  MorphOperator (MorphPlan *morph_plan);

  virtual MorphOperatorView *create_view() = 0;
  virtual const char *type() = 0;
  virtual bool save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;
  virtual void post_load();
  virtual OutputType output_type() = 0;

  MorphPlan *morph_plan();

  std::string name();
  void set_name (const std::string &name);
};

}

#endif
