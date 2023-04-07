// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_SOURCE_MODULE_HH
#define SPECTMORPH_MORPH_SOURCE_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class SimpleWavSetSource : public LiveDecoderSource
{
private:
  WavSet *wav_set;
  Audio  *active_audio;

public:
  SimpleWavSetSource();

  void        set_wav_set (const std::string& path);
  void        set_wav_set (WavSet *wav_set);

  void        retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  Audio      *audio();
  AudioBlock *audio_block (size_t index);
};

class MorphSourceModule : public MorphOperatorModule
{
protected:
  SimpleWavSetSource my_source;

public:
  MorphSourceModule (MorphPlanVoice *voice);
  ~MorphSourceModule();

  void set_config (const MorphOperatorConfig *op_cfg);
  LiveDecoderSource *source();
};
}

#endif
