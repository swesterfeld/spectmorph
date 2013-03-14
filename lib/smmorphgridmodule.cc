// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridmodule.hh"
#include "smmorphgrid.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"
#include "smmath.hh"
#include "smlivedecoder.hh"

#include <assert.h>

using namespace SpectMorph;

using std::min;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphGridModule");

MorphGridModule::MorphGridModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 4)
{
  leak_debugger.add (this);

  width = 0;
  height = 0;

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
}

MorphGridModule::~MorphGridModule()
{
  leak_debugger.del (this);
}

void
MorphGridModule::set_config (MorphOperator *op)
{
  MorphGrid *grid = dynamic_cast<MorphGrid *> (op);

  width = grid->width();
  height = grid->height();

  input_mod.resize (width);

  for (size_t x = 0; x < width; x++)
    {
      input_mod[x].resize (height);
      for (size_t y = 0; y < height; y++)
        {
          MorphOperator *input_op = grid->input_op (x, y);

          if (input_op)
            {
              input_mod[x][y] = morph_plan_voice->module (input_op);
            }
          else
            {
              input_mod[x][y] = NULL;
            }
        }
    }

  x_morphing = grid->x_morphing();
  y_morphing = grid->y_morphing();

  update_dependency (0, input_mod[0][0]);
  update_dependency (1, input_mod[1][0]);
  update_dependency (2, input_mod[0][1]);
  update_dependency (3, input_mod[1][1]);
}

void
MorphGridModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  for (size_t x = 0; x < module->width; x++)
    {
      for (size_t y = 0; y < module->height; y++)
        {
          MorphOperatorModule *input_mod = module->input_mod[x][y];

          if (input_mod && input_mod->source())
            {
              input_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
            }
        }
    }
}

Audio*
MorphGridModule::MySource::audio()
{
  return &module->audio;
}

static bool
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

  if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
    {
      source_index = LiveDecoder::compute_loop_frame_index (source_index, audio);
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

  out_audio_block.lpc_lsf_p = block_ptr->lpc_lsf_p;
  out_audio_block.lpc_lsf_q = block_ptr->lpc_lsf_q;

  return true;
}

namespace
{
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
find_match (float freq, const vector<float>& freqs, const vector<int>& used, size_t *index)
{
  const float lower_bound = freq - 220;
  const float upper_bound = freq + 220;

  double min_diff = 1e20;
  size_t best_index = 0; // initialized to avoid compiler warning

  size_t i = 0;

  // quick scan for beginning of the region containing suitable candidates
  size_t skip = freqs.size() / 2;
  while (skip >= 4)
    {
      while (i + skip < freqs.size() && freqs[i + skip] < lower_bound)
        i += skip;
      skip /= 2;
    }

  while (i < freqs.size() && freqs[i] < upper_bound)
    {
      if (!used[i])
        {
          double diff = fabs (freq - freqs[i]);
          if (diff < min_diff)
            {
              best_index = i;
              min_diff = diff;
            }
        }
      i++;
    }
  if (min_diff < 220)
    {
      *index = best_index;
      return true;
    }
  return false;
}

static void
interp_mag_one (double interp, float *left, float *right)
{
  float l_value = left ? *left : 0;
  float r_value = right ? *right : 0;
#if 0
  if (module->db_linear)
    {
#endif
  // FIXME: handle non-db-linear interps
      double lmag_db = bse_db_from_factor (l_value, -100);
      double rmag_db = bse_db_from_factor (r_value, -100);

      double mag_db = (1 - interp) * lmag_db + interp * rmag_db;
      double mag = bse_db_to_factor (mag_db);

      if (left)
        *left = mag;
      if (right)
        *right = mag;
#if 0
    }
  else
    {
      if (left)
        *left = (1 - interp) * l_value;
      if (right)
        *right = interp * r_value;
    }
#endif
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

}

bool
morph (AudioBlock& out_block,
       bool have_left, const AudioBlock& left_block,
       bool have_right, const AudioBlock& right_block,
       double morphing)
{
  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */

  assert (have_left);
  assert (have_right);

  // clear result block
  out_block.freqs.clear();
  out_block.mags.clear();
  out_block.phases.clear();

  // FIXME: lpc stuff
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

  vector<int> left_used (left_block.freqs.size());
  vector<int> right_used (right_block.freqs.size());
  for (size_t m = 0; m < mds.size(); m++)
    {
      size_t i, j;
      bool match = false;
      if (mds[m].block == MagData::BLOCK_LEFT)
        {
          i = mds[m].index;

          if (!left_used[i])
            match = find_match (left_block.freqs[i], right_block.freqs, right_used, &j);
        }
      else // (mds[m].block == MagData::BLOCK_RIGHT)
        {
          j = mds[m].index;
          if (!right_used[j])
            match = find_match (right_block.freqs[j], left_block.freqs, left_used, &i);
        }
      if (match)
        {
          const double freq =  (1 - interp) * left_block.freqs[i]  + interp * right_block.freqs[j]; // <- NEEDS better averaging
          const double phase = (1 - interp) * left_block.phases[i] + interp * right_block.phases[j];

          // FIXME: prefer freq of louder partial
          // FIXME: lpc
          // FIXME: non-db

          const double lmag_db = bse_db_from_factor (left_block.mags[i], -100);
          const double rmag_db = bse_db_from_factor (right_block.mags[j], -100);
          const double mag_db = (1 - interp) * lmag_db + interp * rmag_db;

          const double mag = bse_db_to_factor (mag_db);
          out_block.freqs.push_back (freq);
          out_block.mags.push_back (mag);
          out_block.phases.push_back (phase);

          left_used[i] = 1;
          right_used[j] = 1;
        }
    }
  for (size_t i = 0; i < left_block.freqs.size(); i++)
    {
      if (!left_used[i])
        {
          out_block.freqs.push_back (left_block.freqs[i]);
          out_block.mags.push_back (left_block.mags[i]);
          out_block.phases.push_back (left_block.phases[i]);

          interp_mag_one (interp, &out_block.mags.back(), NULL);
        }
    }
  for (size_t i = 0; i < right_block.freqs.size(); i++)
    {
      if (!right_used[i])
        {
          out_block.freqs.push_back (right_block.freqs[i]);
          out_block.mags.push_back (right_block.mags[i]);
          out_block.phases.push_back (right_block.phases[i]);

          interp_mag_one (interp, NULL, &out_block.mags.back());
        }
    }
  out_block.noise.clear();
  for (size_t i = 0; i < left_block.noise.size(); i++)
    out_block.noise.push_back ((1 - interp) * left_block.noise[i] + interp * right_block.noise[i]);

  sort_freqs (out_block);
  return true;
}

AudioBlock *
MorphGridModule::MySource::audio_block (size_t index)
{
  assert (module->width == 2);
  assert (module->height == 2);

  AudioBlock audio_block_a, audio_block_b, audio_block_c, audio_block_d;

  /*
   *  A ---- B
   *  |      |
   *  |      |
   *  C ---- D
   */
  bool have_a = get_normalized_block (module->input_mod[0][0]->source(), index, audio_block_a, 0);
  bool have_b = get_normalized_block (module->input_mod[1][0]->source(), index, audio_block_b, 0);
  bool have_c = get_normalized_block (module->input_mod[0][1]->source(), index, audio_block_c, 0);
  bool have_d = get_normalized_block (module->input_mod[1][1]->source(), index, audio_block_d, 0);

  const double x_morphing = module->x_morphing;
  const double y_morphing = module->y_morphing;

  AudioBlock audio_block_ab, audio_block_cd;
  bool have_ab = morph (audio_block_ab, have_a, audio_block_a, have_b, audio_block_b, x_morphing);
  bool have_cd = morph (audio_block_cd, have_c, audio_block_c, have_d, audio_block_d, x_morphing);
  bool have_abcd = morph (module->audio_block, have_ab, audio_block_ab, have_cd, audio_block_cd, y_morphing);

  return have_abcd ? &module->audio_block : NULL;
}

LiveDecoderSource *
MorphGridModule::source()
{
  return &my_source;
}
