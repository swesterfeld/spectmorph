// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperatormodule.hh"
#include "smmorphgrid.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphGridModule : public MorphOperatorModule
{
  std::vector< std::vector<MorphOperatorModule *> > input_mod;

  size_t              width;
  size_t              height;

  double                  x_morphing;
  MorphGrid::ControlType  x_control_type;
  double                  y_morphing;
  MorphGrid::ControlType  y_control_type;

  MorphOperatorModule    *x_control_mod;
  MorphOperatorModule    *y_control_mod;

  Audio               audio;
  AudioBlock          audio_block;

  struct MySource : public LiveDecoderSource
  {
    MorphGridModule *module;

    void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
    Audio* audio();
    AudioBlock *audio_block (size_t index);
  } my_source;

public:
  MorphGridModule (MorphPlanVoice *voice);
  ~MorphGridModule();

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};

}
