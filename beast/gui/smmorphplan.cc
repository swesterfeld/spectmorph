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

#include "smmorphplan.hh"
#include "smmemout.hh"
#include "smoutfile.hh"
#include "smaudio.hh"
#include "smaudio.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

bool
MorphPlan::load_index (const string& filename)
{
  return index.load_file (filename);
}

void
MorphPlan::add_operator (MorphOperator *op)
{
  operators.push_back (op);

  signal_plan_changed();

  vector<unsigned char> data;
  MemOut mo (&data);
  OutFile of (&mo, "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
  for (vector<MorphOperator *>::iterator oi = operators.begin(); oi != operators.end(); oi++)
    {
      of.begin_section ("operator");
      of.end_section();
    }
  for (size_t i = 0; i < data.size(); i++)
    {
      printf ("%02x", data[i]);
    }
  printf ("\n");
  fflush (stdout);
}

const vector<MorphOperator*>&
MorphPlan::get_operators()
{
  return operators;
}
