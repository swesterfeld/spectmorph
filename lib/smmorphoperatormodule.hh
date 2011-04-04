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

#ifndef SPECTMORPH_MORPH_OPERATOR_MODULE_HH
#define SPECTMORPH_MORPH_OPERATOR_MODULE_HH

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"

#include <string>

namespace SpectMorph
{

class MorphPlanVoice;

class MorphOperatorModule
{
protected:
  MorphPlanVoice *morph_plan_voice;

public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual float latency_ms();
  virtual void set_latency_ms (float latency_ms);
  virtual void set_config (MorphOperator *op) = 0;
  virtual LiveDecoderSource *source();

  static MorphOperatorModule *create (MorphOperator *op, MorphPlanVoice *voice);
};

}

#endif
