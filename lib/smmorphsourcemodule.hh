// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphSourceModule : public MorphOperatorModule
{
protected:
  struct MySource : public LiveDecoderSource
  {
    WavSet *wav_set;
    Audio  *active_audio;

    void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
    Audio* audio();
    AudioBlock *audio_block (size_t index);
  } my_source;
public:
  MorphSourceModule (MorphPlanVoice *voice);
  ~MorphSourceModule();

  float latency_ms();
  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};
}
