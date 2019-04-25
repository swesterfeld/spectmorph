// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTENCODER_HH
#define SPECTMORPH_INSTENCODER_HH

#include "smwavdata.hh"
#include "smencoder.hh"
#include "sminstrument.hh"

namespace SpectMorph
{

class InstEncoder
{
  EncoderParams      enc_params;
  std::vector<float> window;

  void setup_params (const WavData& wd, int midi_note);

public:
  Audio *encode (const WavData& wd, int midi_note, Instrument::EncoderConfig& cfg, const std::function<bool()>& kill_function);
};

}

#endif
