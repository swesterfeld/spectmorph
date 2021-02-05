// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_MODULE_HH
#define SPECTMORPH_MORPH_LINEAR_MODULE_HH

#include "smmorphplan.hh"
#include "smmorphoutput.hh"
#include "smmorphoperatormodule.hh"
#include "smmorphlinear.hh"
#include "smmorphsourcemodule.hh"

namespace SpectMorph
{

class MorphLinearModule : public MorphOperatorModule
{
  MorphOperatorModule *left_mod;
  MorphOperatorModule *right_mod;
  MorphOperatorModule *control_mod;
  SimpleWavSetSource   left_source;
  bool                 have_left_source;
  SimpleWavSetSource   right_source;
  bool                 have_right_source;
  float                morphing;
  bool                 db_linear;

  MorphLinear::ControlType control_type;

  Audio                audio;
  AudioBlock           audio_block;

  struct MySource : public LiveDecoderSource
  {
    MorphLinearModule    *module;

    // temporary data for morphing (avoid malloc by putting it here)
    AudioBlock            left_block;
    AudioBlock            right_block;

    void interp_mag_one (double interp, uint16_t *left, uint16_t *right);
    void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
    Audio* audio();
    AudioBlock *audio_block (size_t index);
  } my_source;

public:
  MorphLinearModule (MorphPlanVoice *voice);
  ~MorphLinearModule();

  void set_config (const MorphOperatorConfig *cfg);
  LiveDecoderSource *source();
};

}

#endif
