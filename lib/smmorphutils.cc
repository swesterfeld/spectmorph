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

static bool
fs_cmp (const FreqState& fs1, const FreqState& fs2)
{
  return fs1.freq_f < fs2.freq_f;
}

bool
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

size_t
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


void
init_freq_state (const vector<uint16_t>& fint, FreqState *freq_state)
{
  for (size_t i = 0; i < fint.size(); i++)
    {
      freq_state[i].freq_f = sm_ifreq2freq (fint[i]);
      freq_state[i].used   = 0;
    }
}

void
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

}

}
