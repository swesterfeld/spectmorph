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

#include "smmorphoutputmodule.hh"
#include "smmorphoutput.hh"
#include "smmorphplan.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOutputModule::MorphOutputModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
}

void
MorphOutputModule::set_config (MorphOperator *op)
{
  MorphOutput *out_op = dynamic_cast <MorphOutput *> (op);
  g_return_if_fail (out_op != NULL);

  out_ops.clear();
  out_decoders.clear(); // FIXME: LEAK ?
  for (size_t ch = 0; ch < 4; ch++)
    {
      MorphOperatorModule *mod = NULL;
      LiveDecoder *dec = NULL;

      MorphOperator *op = out_op->channel_op (ch);
      if (op)
        {
          mod = morph_plan_voice->module (op);
          dec = new LiveDecoder (mod->source());
        }

      out_ops.push_back (mod);
      out_decoders.push_back (dec);
    }
}

void
MorphOutputModule::process (size_t n_values, float *values)
{
  out_decoders[0]->process (n_values, 0, 0, values);
}

void
MorphOutputModule::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  out_decoders[0]->retrigger (channel, freq, midi_velocity, mix_freq);
}
