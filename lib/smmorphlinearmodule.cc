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

#define DEBUG (1)

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

bool
get_normalized_block (LiveDecoderSource *source, size_t index, AudioBlock& out_audio_block)
{
  Audio *audio = source->audio();

  if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
    {
      size_t loop_start_index = sm_round_positive (audio->loop_start * 1000.0 / audio->mix_freq);
      size_t loop_end_index   = sm_round_positive (audio->loop_end   * 1000.0 / audio->mix_freq);

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
  int source_index = sm_round_positive (time_ms / audio->frame_step_ms);

  AudioBlock *block_ptr = source->audio_block (source_index);

  if (!block_ptr)
    return false;

  out_audio_block.noise  = block_ptr->noise;
  out_audio_block.mags   = block_ptr->mags;
  out_audio_block.phases = block_ptr->phases;  // usually not used
  out_audio_block.freqs.resize (block_ptr->freqs.size());

  for (size_t i = 0; i < block_ptr->freqs.size(); i++)
    out_audio_block.freqs[i] = block_ptr->freqs[i] * 440 / audio->fundamental_freq;

  return true;
}

struct PartialData
{
  float freq;
  float mag;
  float phase;
};

static bool
pd_cmp (const PartialData& p1, const PartialData& p2)
{
  return p1.freq < p2.freq;
}

static void
sort_freqs (AudioBlock& block)
{
  // sort partials by frequency
  vector<PartialData> pvec;

  for (size_t p = 0; p < block.freqs.size(); p++)
    {
      PartialData pd;
      pd.freq = block.freqs[p];
      pd.mag = block.mags[p];
      pd.phase = block.phases[p];
      pvec.push_back (pd);
    }
  sort (pvec.begin(), pvec.end(), pd_cmp);

  // replace partial data with sorted partial data
  block.freqs.clear();
  block.mags.clear();
  block.phases.clear();

  for (vector<PartialData>::const_iterator pi = pvec.begin(); pi != pvec.end(); pi++)
    {
      block.freqs.push_back (pi->freq);
      block.mags.push_back (pi->mag);
      block.phases.push_back (pi->phase);
    }
}

void
dump_block (size_t index, const char *what, const AudioBlock& block)
{
  if (DEBUG)
    {
      for (size_t i = 0; i < block.freqs.size(); i++)
        printf ("%zd:%s %.17g %.17g\n", index, what, block.freqs[i], block.mags[i]);
    }
}

void
dump_line (size_t index, const char *what, double start, double end)
{
  if (DEBUG)
    {
      printf ("%zd:%s %.17g %.17g\n", index, what, start, end);
    }
}

AudioBlock *
MorphLinearModule::MySource::audio_block (size_t index)
{
  bool have_left = false, have_right = false;

  AudioBlock left_block, right_block;

  if (module->left_mod && module->left_mod->source())
    have_left = get_normalized_block (module->left_mod->source(), index, left_block);

  if (module->right_mod && module->right_mod->source())
    have_right = get_normalized_block (module->right_mod->source(), index, right_block);

  if (have_left && have_right) // true morph: both sources present
    {
      module->audio_block.freqs.clear();
      module->audio_block.mags.clear();
      module->audio_block.phases.clear();

      dump_block (index, "A", left_block);
      dump_block (index, "B", right_block);
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          double min_diff = 1e20;
          size_t best_j = 0; // initialized to avoid compiler warning

          for (size_t j = 0; j < right_block.freqs.size(); j++)
            {
              double diff = fabs (left_block.freqs[i] - right_block.freqs[j]);
              if (diff < min_diff)
                {
                  best_j = j;
                  min_diff = diff;
                }
            }
          if (min_diff < 220)
            {
              double freq = (left_block.freqs[i] + right_block.freqs[best_j]) / 2; // <- NEEDS better averaging
              double mag  = (left_block.mags[i]  + right_block.mags[best_j]) / 2;
              double phase = (left_block.phases[i] + right_block.phases[best_j]) / 2;

              module->audio_block.freqs.push_back (freq);
              module->audio_block.mags.push_back (mag);
              module->audio_block.phases.push_back (phase);
              dump_line (index, "L", left_block.freqs[i], right_block.freqs[best_j]);
              left_block.freqs[i] = 0;
              right_block.freqs[best_j] = 0;
            }
        }
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          if (left_block.freqs[i] != 0)
            {
              module->audio_block.freqs.push_back (left_block.freqs[i]);
              module->audio_block.mags.push_back (left_block.mags[i]);
              module->audio_block.phases.push_back (left_block.phases[i]);
            }
        }
      for (size_t i = 0; i < right_block.freqs.size(); i++)
        {
          if (right_block.freqs[i] != 0)
            {
              module->audio_block.freqs.push_back (right_block.freqs[i]);
              module->audio_block.mags.push_back (right_block.mags[i]);
              module->audio_block.phases.push_back (right_block.phases[i]);
            }
        }
      assert (left_block.noise.size() == right_block.noise.size());

      module->audio_block.noise.clear();
      for (size_t i = 0; i < left_block.noise.size(); i++)
        {
          module->audio_block.noise.push_back ((left_block.noise[i] + right_block.noise[i]) / 2);
        }
      sort_freqs (module->audio_block);

      return &module->audio_block;
    }
  else if (have_left) // only left source output present
    {
      module->audio_block = left_block;

      return &module->audio_block;
    }
  else if (have_right) // only right source output present
    {
      module->audio_block = right_block;

      return &module->audio_block;
    }
  return NULL;
}

LiveDecoderSource *
MorphLinearModule::source()
{
  return &my_source;
}
