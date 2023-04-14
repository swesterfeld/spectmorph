// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"
#include "smproject.hh"
#include "smmorphwavsource.hh"
#include <memory>

namespace SpectMorph
{

class MorphWavSourceModule;

class MorphWavSourceModule : public MorphOperatorModule
{
  const MorphWavSource::Config *cfg = nullptr;

  class InstrumentSource : public LiveDecoderSource
  {
    Audio                  *active_audio = nullptr;
    std::shared_ptr<WavSet> wav_set;
    int                     object_id;
    Project                *project;
  public:
    MorphWavSourceModule   *module = nullptr;

    void retrigger (int channel, float freq, int midi_velocity) override;
    Audio *audio() override;
    AudioBlock *audio_block (size_t index) override;

    void update_project (Project *project);
    void update_object_id (int object_id);
  };

  InstrumentSource my_source;

public:
  MorphWavSourceModule (MorphPlanVoice *voice);
  ~MorphWavSourceModule();

  void set_config (const MorphOperatorConfig *op_cfg);
  LiveDecoderSource *source();
};

}

#endif
