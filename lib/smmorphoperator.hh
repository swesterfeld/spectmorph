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

#include <QObject>

namespace SpectMorph
{

class MorphOperatorView;
class MorphPlan;

class MorphOperator : public QObject
{
  Q_OBJECT
protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;
  std::string m_id;

  void write_operator (OutFile& file, const std::string& name, MorphOperator *op);

public:
  enum OutputType {
    OUTPUT_NONE,
    OUTPUT_AUDIO,
    OUTPUT_CONTROL
  };
  MorphOperator (MorphPlan *morph_plan);
  virtual ~MorphOperator();

  virtual const char *type() = 0;
  virtual bool save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;
  virtual void post_load();
  virtual OutputType output_type() = 0;

  MorphPlan *morph_plan();

  std::string type_name();

  std::string name();
  void set_name (const std::string& name);

  bool can_rename (const std::string& name);

  std::string id();
  void set_id (const std::string& id);

  static MorphOperator *create (const std::string& type, MorphPlan *plan);
};

}

#endif
