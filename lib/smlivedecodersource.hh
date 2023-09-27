// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_LIVEDECODER_SOURCE_HH
#define SPECTMORPH_LIVEDECODER_SOURCE_HH

#include "smaudio.hh"
#include "smrtmemory.hh"

namespace SpectMorph {

class LiveDecoderSource
{
public:
  virtual void retrigger (int channel, float freq, int midi_velocity) = 0;
  virtual Audio *audio() = 0;
  virtual AudioBlock *audio_block (size_t index) = 0;
  virtual bool rt_audio_block (size_t index, RTAudioBlock& rt_audio_block) { return false; }
  virtual ~LiveDecoderSource();
};

}
#endif
