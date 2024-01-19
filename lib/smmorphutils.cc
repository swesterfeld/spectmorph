// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphutils.hh"
#include "smmath.hh"

#include <algorithm>

using std::vector;
using std::sort;
using std::min;
using std::max;

namespace SpectMorph
{

namespace MorphUtils
{

struct MagData
{
  enum {
    BLOCK_LEFT  = 0,
    BLOCK_RIGHT = 1
  }        block;
  size_t   index;
  uint16_t mag;
};

static inline bool
md_cmp (const MagData& m1, const MagData& m2)
{
  return m1.mag > m2.mag;  // sort with biggest magnitude first
}

struct FreqState
{
  float freq_f;
  int   used;
};

static bool
fs_cmp (const FreqState& fs1, const FreqState& fs2)
{
  return fs1.freq_f < fs2.freq_f;
}

static bool
find_match (float freq, const FreqState *freq_state, size_t freq_state_size, size_t *index)
{
  const float freq_start = freq - 0.5;
  const float freq_end   = freq + 0.5;

  double min_diff = 1e20;
  size_t best_index = 0; // initialized to avoid compiler warning

  FreqState start_freq_state = {freq_start, 0};
  const FreqState *start_ptr = std::lower_bound (freq_state, freq_state + freq_state_size, start_freq_state, fs_cmp);
  size_t i = start_ptr - freq_state;

  while (i < freq_state_size && freq_state[i].freq_f < freq_end)
    {
      if (!freq_state[i].used)
        {
          double diff = fabs (freq - freq_state[i].freq_f);
          if (diff < min_diff)
            {
              best_index = i;
              min_diff = diff;
            }
        }
      i++;
    }
  if (min_diff < 0.5)
    {
      *index = best_index;
      return true;
    }
  return false;
}

static size_t
init_mag_data (MagData *mds, const RTAudioBlock& left_block, const RTAudioBlock& right_block)
{
  size_t mds_size = 0;
  for (size_t i = 0; i < left_block.freqs.size(); i++)
    {
      MagData& md = mds[mds_size];

      md.block = MagData::BLOCK_LEFT;
      md.index = i;
      md.mag   = left_block.mags[i];
      mds_size++;
    }
  for (size_t i = 0; i < right_block.freqs.size(); i++)
    {
      MagData& md = mds[mds_size];

      md.block = MagData::BLOCK_RIGHT;
      md.index = i;
      md.mag   = right_block.mags[i];
      mds_size++;
    }
  sort (mds, mds + mds_size, md_cmp);
  return mds_size;
}


static void
init_freq_state (const RTVector<uint16_t>& fint, FreqState *freq_state)
{
  for (size_t i = 0; i < fint.size(); i++)
    {
      freq_state[i].freq_f = sm_ifreq2freq (fint[i]);
      freq_state[i].used   = 0;
    }
}

void
interp_mag_one (double interp, uint16_t *left, uint16_t *right, MorphMode mode)
{
  if (mode == MorphMode::DB_LINEAR)
    {
      const uint16_t lmag_idb = max<uint16_t> (left ? *left : 0, SM_IDB_CONST_M96);
      const uint16_t rmag_idb = max<uint16_t> (right ? *right : 0, SM_IDB_CONST_M96);

      const uint16_t mag_idb = sm_round_positive ((1 - interp) * lmag_idb + interp * rmag_idb);

      if (left)
        *left = mag_idb;
      if (right)
        *right = mag_idb;
    }
  else
    {
      if (left)
        *left = sm_factor2idb ((1 - interp) * sm_idb2factor (*left));
      if (right)
        *right = sm_factor2idb (interp * sm_idb2factor (*right));
    }
}

void
morph_scale (RTAudioBlock& out_block, const RTAudioBlock& in_block, double factor, MorphUtils::MorphMode mode)
{
  const int ddb = sm_factor2delta_idb (factor);

  out_block.assign (in_block);
  for (size_t i = 0; i < out_block.noise.size(); i++)
    out_block.noise[i] = sm_bound<int> (0, out_block.noise[i] + ddb, 65535);

  for (size_t i = 0; i < out_block.freqs.size(); i++)
    interp_mag_one (factor, NULL, &out_block.mags[i], mode);
}

bool
get_normalized_block (LiveDecoderSource *source, double time_ms, RTAudioBlock& out_audio_block)
{
  if (!source)
    return false;

  Audio *audio = source->audio();
  if (!audio)
    return false;

  if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
    {
      const double loop_start_ms = audio->loop_start * 1000.0 / audio->mix_freq;
      const double loop_end_ms   = audio->loop_end   * 1000.0 / audio->mix_freq;

      if (loop_start_ms >= loop_end_ms)
        {
          /* loop_start_index usually should be less than loop_end_index, this is just
           * to handle corner cases and pathological cases
           */
          time_ms = min (time_ms, loop_start_ms);
        }
      else if (time_ms > loop_end_ms)
        {
          /* compute loop position: ensure that time_ms is in [loop_start_ms, loop_end_ms] */
          time_ms = fmod (time_ms - loop_start_ms, loop_end_ms - loop_start_ms) + loop_start_ms;
        }
    }

  int source_index = sm_round_positive (time_ms / audio->frame_step_ms);
  if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
    {
      source_index = LiveDecoder::compute_loop_frame_index (source_index, audio);
    }

  return source->rt_audio_block (source_index, out_audio_block);
}

bool
morph (RTAudioBlock& out_block,
       bool have_left, const RTAudioBlock& left_block,
       bool have_right, const RTAudioBlock& right_block,
       double morphing, MorphUtils::MorphMode morph_mode)
{
  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */

  if (!have_left && !have_right) // nothing + nothing = nothing
    return false;

  if (!have_left) // nothing + interp * right = interp * right
    {
      MorphUtils::morph_scale (out_block, right_block, interp, morph_mode);
      return true;
    }
  if (!have_right) // (1 - interp) * left + nothing = (1 - interp) * left
    {
      MorphUtils::morph_scale (out_block, left_block, 1 - interp, morph_mode);
      return true;
    }

  // set out_block capacity
  const size_t max_partials = left_block.freqs.size() + right_block.freqs.size();
  out_block.freqs.set_capacity (max_partials);
  out_block.mags.set_capacity (max_partials);

  MagData mds[max_partials + AVOID_ARRAY_UB];

  size_t mds_size = MorphUtils::init_mag_data (mds, left_block, right_block);
  size_t left_freqs_size = left_block.freqs.size();
  size_t right_freqs_size = right_block.freqs.size();

  MorphUtils::FreqState   left_freqs[left_freqs_size + AVOID_ARRAY_UB];
  MorphUtils::FreqState   right_freqs[right_freqs_size + AVOID_ARRAY_UB];

  init_freq_state (left_block.freqs, left_freqs);
  init_freq_state (right_block.freqs, right_freqs);

  for (size_t m = 0; m < mds_size; m++)
    {
      size_t i, j;
      bool match = false;
      if (mds[m].block == MagData::BLOCK_LEFT)
        {
          i = mds[m].index;

          if (!left_freqs[i].used)
            match = MorphUtils::find_match (left_freqs[i].freq_f, right_freqs, right_freqs_size, &j);
        }
      else // (mds[m].block == MagData::BLOCK_RIGHT)
        {
          j = mds[m].index;
          if (!right_freqs[j].used)
            match = MorphUtils::find_match (right_freqs[j].freq_f, left_freqs, left_freqs_size, &i);
        }
      if (match)
        {
          /* prefer frequency of louder partial:
           *
           * if the magnitudes are similar, mfact will be close to 1, and freq will become approx.
           *
           *   freq = (1 - interp) * lfreq + interp * rfreq
           *
           * if the magnitudes are very different, mfact will be close to 0, and freq will become
           *
           *   freq ~= lfreq         // if left partial is louder
           *   freq ~= rfreq         // if right partial is louder
           */
          const double lfreq = left_block.freqs[i];
          const double rfreq = right_block.freqs[j];
          double freq;

          if (left_block.mags[i] > right_block.mags[j])
            {
              const double mfact = right_block.mags_f (j) / left_block.mags_f (i);

              freq = lfreq + mfact * interp * (rfreq - lfreq);
            }
          else
            {
              const double mfact = left_block.mags_f (i) / right_block.mags_f (j);

              freq = rfreq + mfact * (1 - interp) * (lfreq - rfreq);
            }

          uint16_t mag_idb;
          if (morph_mode == MorphUtils::MorphMode::DB_LINEAR)
            {
              const uint16_t lmag_idb = max (left_block.mags[i], SM_IDB_CONST_M96);
              const uint16_t rmag_idb = max (right_block.mags[j], SM_IDB_CONST_M96);

              mag_idb = sm_round_positive ((1 - interp) * lmag_idb + interp * rmag_idb);
            }
          else
            {
              mag_idb = sm_factor2idb ((1 - interp) * left_block.mags_f (i) + interp * right_block.mags_f (j));
            }
          out_block.freqs.push_back (freq);
          out_block.mags.push_back (mag_idb);

          left_freqs[i].used = 1;
          right_freqs[j].used = 1;
        }
    }
  for (size_t i = 0; i < left_block.freqs.size(); i++)
    {
      if (!left_freqs[i].used)
        {
          out_block.freqs.push_back (left_block.freqs[i]);
          out_block.mags.push_back (left_block.mags[i]);

          interp_mag_one (interp, &out_block.mags.back(), NULL, morph_mode);
        }
    }
  for (size_t i = 0; i < right_block.freqs.size(); i++)
    {
      if (!right_freqs[i].used)
        {
          out_block.freqs.push_back (right_block.freqs[i]);
          out_block.mags.push_back (right_block.mags[i]);

          interp_mag_one (interp, NULL, &out_block.mags.back(), morph_mode);
        }
    }
  assert (left_block.noise.size() == right_block.noise.size());

  out_block.noise.set_capacity (left_block.noise.size());
  for (size_t i = 0; i < left_block.noise.size(); i++)
    out_block.noise.push_back (sm_factor2idb ((1 - interp) * left_block.noise_f (i) + interp * right_block.noise_f (i)));

  out_block.sort_freqs();

  return true;
}

}

}
