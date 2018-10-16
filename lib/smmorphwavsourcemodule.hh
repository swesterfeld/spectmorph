// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"
#include <memory>

namespace SpectMorph
{

class InstrumentSource : public LiveDecoderSource
{
  Audio                  *active_audio = nullptr;
  std::shared_ptr<WavSet> wav_set;
  std::string             instrument;
public:
  void retrigger (int channel, float freq, int midi_velocity, float mix_freq) override;
  Audio *audio() override;
  AudioBlock *audio_block (size_t index) override;

  void update_instrument (const std::string& instrument);
};

class MorphWavSourceModule : public MorphOperatorModule
{
  InstrumentSource my_source;

public:
  MorphWavSourceModule (MorphPlanVoice *voice);
  ~MorphWavSourceModule();

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};

}

#endif
