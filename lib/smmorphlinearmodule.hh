// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  const MorphLinear::Config *cfg = nullptr;

  MorphOperatorModule *left_mod;
  MorphOperatorModule *right_mod;
  SimpleWavSetSource   left_source;
  bool                 have_left_source;
  SimpleWavSetSource   right_source;
  bool                 have_right_source;

  Audio                audio;

  struct MySource : public LiveDecoderSource
  {
    MorphLinearModule    *module;

    void interp_mag_one (double interp, uint16_t *left, uint16_t *right);
    void retrigger (int channel, float freq, int midi_velocity) override;
    Audio* audio() override;
    AudioBlock *audio_block (size_t index) override { return nullptr; }
    bool rt_audio_block (size_t index, RTAudioBlock& block) override;
  } my_source;

public:
  MorphLinearModule (MorphPlanVoice *voice);
  ~MorphLinearModule();

  void set_config (const MorphOperatorConfig *cfg);
  LiveDecoderSource *source();
};

}

#endif
