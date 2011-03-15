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

#include "smmorphoperatormodule.hh"

#include "smmorphlinearmodule.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphsourcemodule.hh"

using namespace SpectMorph;

using std::string;

MorphOperatorModule::MorphOperatorModule (MorphPlanVoice *voice) :
  morph_plan_voice (voice)
{
}

LiveDecoderSource *
MorphOperatorModule::source()
{
  return NULL;
}

MorphOperatorModule*
MorphOperatorModule::create (MorphOperator *op, MorphPlanVoice *voice)
{
  string type = op->type();

  if (type == "SpectMorph::MorphLinear") return new MorphLinearModule (voice);
  if (type == "SpectMorph::MorphSource") return new MorphSourceModule (voice);
  if (type == "SpectMorph::MorphOutput") return new MorphOutputModule (voice);

  return NULL;
}


