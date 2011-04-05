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
#include "smleakdebugger.hh"
#include <glib.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::min;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphLinearModule");

#define DEBUG (0)

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
  audio.start_ms             = 0;

  left_delay_blocks = 0;
  right_delay_blocks = 0;

  leak_debugger.add (this);
}

MorphLinearModule::~MorphLinearModule()
{
  leak_debugger.del (this);
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
  control_type = linear->control_type();
}

void
MorphLinearModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  float new_start_ms = 0;
  Audio *audio_left = NULL;
  Audio *audio_right = NULL;

  if (module->left_mod && module->left_mod->source())
    {
      module->left_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
      audio_left = module->left_mod->source()->audio();
      if (audio_left)
        new_start_ms = max (new_start_ms, audio_left->start_ms);
    }

  if (module->right_mod && module->right_mod->source())
    {
      module->right_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
      audio_right = module->right_mod->source()->audio();
      if (audio_right)
        new_start_ms = max (new_start_ms, audio_right->start_ms);
    }

  if (audio_left)
    module->left_delay_blocks = max (sm_round_positive (new_start_ms - audio_left->start_ms), 0);
  else
    module->left_delay_blocks = 0;

  if (audio_right)
    module->right_delay_blocks = max (sm_round_positive (new_start_ms - audio_right->start_ms), 0);
  else
    module->right_delay_blocks = 0;

  module->audio.start_ms = new_start_ms;
}

Audio*
MorphLinearModule::MySource::audio()
{
  return &module->audio;
}

bool
get_normalized_block (LiveDecoderSource *source, size_t index, AudioBlock& out_audio_block, int delay_blocks)
{
  g_return_val_if_fail (delay_blocks >= 0, false);
  if (index < delay_blocks)
    {
      out_audio_block.noise.resize (32);
      return true;  // morph with empty block to ensure correct time alignment
    }
  else
    {
      // shift audio for time alignment
      assert (index >= delay_blocks);
      index -= delay_blocks;
    }

  Audio *audio = source->audio();
  if (!audio)
    return false;

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

  if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
    {
      if (audio->loop_start != -1 && source_index > audio->loop_start)
        {
          source_index = audio->loop_start;
        }
    }

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

struct MagData
{
  enum {
    BLOCK_LEFT  = 0,
    BLOCK_RIGHT = 1
  }      block;
  size_t index;
  double mag;
};

static bool
md_cmp (const MagData& m1, const MagData& m2)
{
  return m1.mag > m2.mag;  // sort with biggest magnitude first
}

static bool
find_match (float freq, vector<float>& freqs, size_t *index)
{
  double min_diff = 1e20;
  size_t best_index = 0; // initialized to avoid compiler warning

  for (size_t i = 0; i < freqs.size(); i++)
    {
      double diff = fabs (freq - freqs[i]);
      if (diff < min_diff)
        {
          best_index = i;
          min_diff = diff;
        }
    }
  if (min_diff < 220 && (freq > 1) && (freqs[best_index] > 1))
    {
      *index = best_index;
      return true;
    }
  return false;
}

AudioBlock *
MorphLinearModule::MySource::audio_block (size_t index)
{
  bool have_left = false, have_right = false;

  double morphing;

  if (module->control_type == MorphLinear::CONTROL_GUI)
    morphing = module->morphing;
  else if (module->control_type == MorphLinear::CONTROL_SIGNAL_1)
    morphing = module->morph_plan_voice->control_input (0);
  else if (module->control_type == MorphLinear::CONTROL_SIGNAL_2)
    morphing = module->morph_plan_voice->control_input (1);
  else
    g_assert_not_reached();

  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */

  AudioBlock left_block, right_block;

  if (module->left_mod && module->left_mod->source())
    have_left = get_normalized_block (module->left_mod->source(), index, left_block, module->left_delay_blocks);

  if (module->right_mod && module->right_mod->source())
    have_right = get_normalized_block (module->right_mod->source(), index, right_block, module->right_delay_blocks);

  if (have_left && have_right) // true morph: both sources present
    {
      module->audio_block.freqs.clear();
      module->audio_block.mags.clear();
      module->audio_block.phases.clear();

      dump_block (index, "A", left_block);
      dump_block (index, "B", right_block);

      vector<MagData> mds;
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          MagData md = { MagData::BLOCK_LEFT, i, left_block.mags[i] };
          mds.push_back (md);
        }
      for (size_t i = 0; i < right_block.freqs.size(); i++)
        {
          MagData md = { MagData::BLOCK_RIGHT, i, right_block.mags[i] };
          mds.push_back (md);
        }
      sort (mds.begin(), mds.end(), md_cmp);
      for (size_t m = 0; m < mds.size(); m++)
        {
          size_t i, j;
          bool match;
          if (mds[m].block == MagData::BLOCK_LEFT)
            {
              i = mds[m].index;
              match = find_match (left_block.freqs[i], right_block.freqs, &j);
            }
          else // (mds[m].block == MagData::BLOCK_RIGHT)
            {
              j = mds[m].index;
              match = find_match (right_block.freqs[j], left_block.freqs, &i);
            }
          if (match)
            {
              double freq =  (1 - interp) * left_block.freqs[i]  + interp * right_block.freqs[j]; // <- NEEDS better averaging
              double mag  =  (1 - interp) * left_block.mags[i]   + interp * right_block.mags[j];
              double phase = (1 - interp) * left_block.phases[i] + interp * right_block.phases[j];

              module->audio_block.freqs.push_back (freq);
              module->audio_block.mags.push_back (mag);
              module->audio_block.phases.push_back (phase);
              dump_line (index, "L", left_block.freqs[i], right_block.freqs[j]);
              left_block.freqs[i] = 0;
              right_block.freqs[j] = 0;
            }
        }
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          if (left_block.freqs[i] != 0)
            {
              module->audio_block.freqs.push_back (left_block.freqs[i]);
              module->audio_block.mags.push_back ((1 - interp) * left_block.mags[i]);
              module->audio_block.phases.push_back (left_block.phases[i]);
            }
        }
      for (size_t i = 0; i < right_block.freqs.size(); i++)
        {
          if (right_block.freqs[i] != 0)
            {
              module->audio_block.freqs.push_back (right_block.freqs[i]);
              module->audio_block.mags.push_back (interp * right_block.mags[i]);
              module->audio_block.phases.push_back (right_block.phases[i]);
            }
        }
      assert (left_block.noise.size() == right_block.noise.size());

      module->audio_block.noise.clear();
      for (size_t i = 0; i < left_block.noise.size(); i++)
        module->audio_block.noise.push_back ((1 - interp) * left_block.noise[i] + interp * right_block.noise[i]);

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
