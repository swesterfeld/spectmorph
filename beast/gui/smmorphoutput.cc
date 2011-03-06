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

#include "smmorphoutput.hh"
#include "smmorphoutputview.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

using std::string;

MorphOutput::MorphOutput (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
}

MorphOperatorView *
MorphOutput::create_view()
{
  return new MorphOutputView (this);
}

const char *
MorphOutput::type()
{
  return "SpectMorph::MorphOutput";
}

bool
MorphOutput::save (OutFile& out_file)
{
  return true;
}

bool
MorphOutput::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      ifile.next_event();
    }
  return true;
}
