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

enum class MorphMode {
  LINEAR,
  DB_LINEAR
};

void interp_mag_one (double interp, uint16_t *left, uint16_t *right, MorphMode mode);
void morph_scale (RTAudioBlock& out_block, const RTAudioBlock& in_block, double factor, MorphMode mode);
bool morph (RTAudioBlock& out_block,
            bool have_left, const RTAudioBlock& left_block,
            bool have_right, const RTAudioBlock& right_block,
            double morphing, MorphUtils::MorphMode morph_mode);

AudioBlock* get_normalized_block_ptr (LiveDecoderSource *source, double time_ms);
bool get_normalized_block (LiveDecoderSource *source, double time_ms, RTAudioBlock& out_audio_block);

}

}

#endif
