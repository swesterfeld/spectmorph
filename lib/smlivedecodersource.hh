// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_LIVEDECODER_SOURCE_HH
#define SPECTMORPH_LIVEDECODER_SOURCE_HH

#include "smaudio.hh"

namespace SpectMorph {

class LiveDecoderSource
{
public:
  virtual void retrigger (int channel, float freq, int midi_velocity, float mix_freq) = 0;
  virtual Audio *audio() = 0;
  virtual AudioBlock *audio_block (size_t index) = 0;
  virtual ~LiveDecoderSource();
};

}
#endif
