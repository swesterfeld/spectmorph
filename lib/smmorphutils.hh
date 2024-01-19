// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_UTILS_HH
#define SPECTMORPH_MORPH_UTILS_HH

#include <algorithm>
#include <vector>
#include <stdint.h>

#include "smaudio.hh"
#include "smlivedecoder.hh"
#include "smrtmemory.hh"

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


enum class MorphMode {
  LINEAR,
  DB_LINEAR
};

bool find_match (float freq, const FreqState *freq_state, size_t freq_state_size, size_t *index);
size_t init_mag_data (MagData *mds, const RTAudioBlock& left_block, const RTAudioBlock& right_block);
void init_freq_state (const std::vector<uint16_t>& fint, FreqState *freq_state);
void init_freq_state (const RTVector<uint16_t>& fint, FreqState *freq_state);
void interp_mag_one (double interp, uint16_t *left, uint16_t *right, MorphMode mode);
void morph_scale (RTAudioBlock& out_block, const RTAudioBlock& in_block, double factor, MorphMode mode);

AudioBlock* get_normalized_block_ptr (LiveDecoderSource *source, double time_ms);
bool get_normalized_block (LiveDecoderSource *source, double time_ms, RTAudioBlock& out_audio_block);

}

}

#endif
