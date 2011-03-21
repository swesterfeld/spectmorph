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

#include "smmorphlinear.hh"
#include "smmorphplan.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphLinear::MorphLinear (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  morph_plan->signal_operator_removed.connect (sigc::mem_fun (*this, &MorphLinear::on_operator_removed));

  m_left_op = NULL;
  m_right_op = NULL;
  m_morphing = 0;
  m_control_type = CONTROL_GUI;
}

const char *
MorphLinear::type()
{
  return "SpectMorph::MorphLinear";
}

bool
MorphLinear::save (OutFile& out_file)
{
  write_operator (out_file, "left", m_left_op);
  write_operator (out_file, "right", m_right_op);
  out_file.write_float ("morphing", m_morphing);
  out_file.write_int ("control_type", m_control_type);

  return true;
}

bool
MorphLinear::load (InFile& ifile)
{
  load_left = "";
  load_right = "";

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "left")
            {
              load_left = ifile.event_data();
            }
          else if (ifile.event_name() == "right")
            {
              load_right = ifile.event_data();
            }
          else
            {
              g_printerr ("bad string\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (ifile.event_name() == "morphing")
            {
              m_morphing = ifile.event_float();
            }
          else
            {
              g_printerr ("bad float\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "control_type")
            {
              m_control_type = static_cast<ControlType> (ifile.event_int());
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else
        {
          g_printerr ("bad event\n");
          return false;
        }
      ifile.next_event();
    }
  return true;
}

void
MorphLinear::post_load()
{
  const vector<MorphOperator *>& ops = m_morph_plan->operators();

  m_left_op = NULL;
  m_right_op = NULL;

  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *morph_op = *oi;
      if (morph_op->name() == load_left)
        m_left_op = morph_op;
      if (morph_op->name() == load_right)
        m_right_op = morph_op;
    }
}

MorphOperator::OutputType
MorphLinear::output_type()
{
  return OUTPUT_AUDIO;
}

void
MorphLinear::on_operator_removed (MorphOperator *op)
{
}

MorphOperator *
MorphLinear::left_op()
{
  return m_left_op;
}

MorphOperator *
MorphLinear::right_op()
{
  return m_right_op;
}

void
MorphLinear::set_left_op (MorphOperator *op)
{
  m_left_op = op;

  m_morph_plan->emit_plan_changed();
}

void
MorphLinear::set_right_op (MorphOperator *op)
{
  m_right_op = op;

  m_morph_plan->emit_plan_changed();
}

double
MorphLinear::morphing()
{
  return m_morphing;
}

void
MorphLinear::set_morphing (double new_morphing)
{
  m_morphing = new_morphing;

  m_morph_plan->emit_plan_changed();
}

MorphLinear::ControlType
MorphLinear::control_type()
{
  return m_control_type;
}

void
MorphLinear::set_control_type (ControlType new_control_type)
{
  m_control_type = new_control_type;

  m_morph_plan->emit_plan_changed();
}
