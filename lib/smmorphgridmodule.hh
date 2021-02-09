// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_MODULE_HH
#define SPECTMORPH_MORPH_GRID_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphgrid.hh"
#include "smwavset.hh"
#include "smmorphsourcemodule.hh"

namespace SpectMorph
{

class MorphGridModule : public MorphOperatorModule
{
public:
  struct InputNode
  {
    MorphOperatorModule *mod;
    double               delta_db;
    bool                 has_source;
    SimpleWavSetSource   source;
  };

private:
  const MorphGrid::Config *cfg = nullptr;

  std::vector< std::vector<InputNode> > input_node;

  MorphOperatorModule    *x_control_mod;
  MorphOperatorModule    *y_control_mod;

  // output
  Audio               audio;
  AudioBlock          audio_block;

  struct MySource : public LiveDecoderSource
  {
    // temporary blocks for morphing:
    AudioBlock        audio_block_a;
    AudioBlock        audio_block_b;
    AudioBlock        audio_block_c;
    AudioBlock        audio_block_d;
    AudioBlock        audio_block_ab;
    AudioBlock        audio_block_cd;

    MorphGridModule  *module;

    void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
    Audio* audio();
    AudioBlock *audio_block (size_t index);
  } my_source;

public:
  MorphGridModule (MorphPlanVoice *voice);
  ~MorphGridModule();

  void set_config (const MorphOperatorConfig *cfg);
  LiveDecoderSource *source();
};

}

#endif
