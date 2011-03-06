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

#include "smmorphoperator.hh"

using namespace SpectMorph;

using std::string;

MorphOperator::MorphOperator (MorphPlan *morph_plan) :
  m_morph_plan (morph_plan)
{
}

MorphPlan*
MorphOperator::morph_plan()
{
  return m_morph_plan;
}

string
MorphOperator::name()
{
  return m_name;
}

void
MorphOperator::set_name (const string& name)
{
  m_name = name;
}

void
MorphOperator::post_load()
{
  // override this for post load notification
}
