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

bool find_match (float freq, const FreqState *freq_state, size_t freq_state_size, size_t *index);
void init_freq_state (const std::vector<uint16_t>& fint, FreqState *freq_state);
void init_freq_state (const RTVector<uint16_t>& fint, FreqState *freq_state);

AudioBlock* get_normalized_block_ptr (LiveDecoderSource *source, double time_ms);
bool get_normalized_block (LiveDecoderSource *source, double time_ms, RTAudioBlock& out_audio_block);

}

}

#endif
