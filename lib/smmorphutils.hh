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

bool morph (RTAudioBlock& out_block,
            bool have_left, const RTAudioBlock& left_block,
            bool have_right, const RTAudioBlock& right_block,
            double morphing, MorphUtils::MorphMode morph_mode);

bool get_normalized_block (LiveDecoderSource *source, double time_ms, RTAudioBlock& out_audio_block);

}

}

#endif
