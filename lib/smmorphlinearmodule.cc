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
#include "smmath.hh"
#include <glib.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::min;

MorphLinearModule::MorphLinearModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;

  audio.fundamental_freq     = 440;
  audio.mix_freq             = 48000;
  audio.frame_size_ms        = 1;
  audio.frame_step_ms        = 1;
  audio.attack_start_ms      = 0;
  audio.attack_end_ms        = 0;
  audio.zeropad              = 4;
  audio.loop_type            = Audio::LOOP_NONE;
  audio.zero_values_at_start = 0;
  audio.sample_count         = 2 << 31;
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
  return &module->audio;
}

AudioBlock *
MorphLinearModule::MySource::audio_block (size_t index)
{
  if (module->left_mod && module->left_mod->source())
    {
      Audio *left_audio = module->left_mod->source()->audio();

      if (left_audio->loop_type == Audio::LOOP_TIME_FORWARD)
        {
          size_t loop_start_index = sm_round_positive (left_audio->loop_start * 1000.0 / left_audio->mix_freq);
          size_t loop_end_index   = sm_round_positive (left_audio->loop_end   * 1000.0 / left_audio->mix_freq);

          if (loop_start_index >= loop_end_index)
            {
              /* loop_start_index usually should be less than loop_end_index, this is just
               * to handle corner cases and pathological cases
               */
              index = min (index, loop_start_index);
            }
          else
            {
              while (index >= loop_end_index)
                {
                  index -= (loop_end_index - loop_start_index);
                }
            }
        }

      double time_ms = index; // 1ms frame step
      int left_index = sm_round_positive (time_ms / left_audio->frame_step_ms);

      AudioBlock *left_block_ptr = module->left_mod->source()->audio_block (left_index);

      if (left_block_ptr)
        {
          module->audio_block.noise = left_block_ptr->noise;
          module->audio_block.mags  = left_block_ptr->mags;
          module->audio_block.phases = left_block_ptr->phases;  // usually not used
          module->audio_block.freqs.resize (left_block_ptr->freqs.size());

          for (size_t i = 0; i < left_block_ptr->freqs.size(); i++)
            {
              module->audio_block.freqs[i] = left_block_ptr->freqs[i] * 440 / left_audio->fundamental_freq;
            }
          return &module->audio_block;
        }
      else
        return NULL;
    }

  return NULL;
}

LiveDecoderSource *
MorphLinearModule::source()
{
  return &my_source;
}
