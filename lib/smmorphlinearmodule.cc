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

#include "smmorphlinearmodule.hh"
#include "smmorphlinear.hh"
#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphLinearModule::MorphLinearModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;
}

void
MorphLinearModule::set_config (MorphOperator *op)
{
  MorphLinear *linear = dynamic_cast<MorphLinear *> (op);
  MorphOperator *left_op = linear->left_op();
  MorphOperator *right_op = linear->right_op();

  if (left_op)
    left_mod = morph_plan_voice->module (left_op);
  else
    left_mod = NULL;

  if (right_op)
    right_mod = morph_plan_voice->module (right_op);
  else
    right_mod = NULL;

  morphing = linear->morphing();
}

void
MorphLinearModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  if (module->left_mod && module->left_mod->source())
    module->left_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);

  if (module->right_mod && module->right_mod->source())
    module->right_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
}

Audio*
MorphLinearModule::MySource::audio()
{
  if (module->left_mod && module->left_mod->source())
    return module->left_mod->source()->audio();

  return NULL;
}

AudioBlock *
MorphLinearModule::MySource::audio_block (size_t index)
{
  if (module->left_mod && module->left_mod->source())
    return module->left_mod->source()->audio_block (index);

  return NULL;
}

LiveDecoderSource *
MorphLinearModule::source()
{
  return &my_source;
}
