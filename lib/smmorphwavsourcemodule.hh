// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"
#include "smproject.hh"
#include "smmorphwavsource.hh"
#include "smformantcorrection.hh"
#include <memory>

namespace SpectMorph
{

class MorphWavSourceModule;

class MorphWavSourceModule : public MorphOperatorModule
{
  LeakDebugger leak_debugger { "SpectMorph::MorphWavSourceModule" };

  const MorphWavSource::Config *cfg = nullptr;

  class InstrumentSource : public LiveDecoderSource
  {
    FormantCorrection       formant_correction;
    Audio                  *active_audio = nullptr;
    int                     object_id = 0;
    Project                *project = nullptr;
  public:
    MorphWavSourceModule   *module = nullptr;
    double                  last_time_ms = 0;

    void
    set_portamento_freq (float freq) override
    {
      if (active_audio)
        formant_correction.set_ratio (freq / active_audio->fundamental_freq);
    }
    void update_voice_source (const MorphWavSource::Config *cfg);
    void retrigger (int channel, float freq, int midi_velocity) override;
    Audio *audio() override;
    bool rt_audio_block (size_t index, RTAudioBlock& out_block) override;

    void update_project_and_object_id (Project *project, int object_id);
  };

  InstrumentSource my_source;

public:
  MorphWavSourceModule (MorphPlanVoice *voice);

  void set_config (const MorphOperatorConfig *op_cfg) override;
  void note_on (const TimeInfo& time_info) override;
  LiveDecoderSource *source() override;
};

}

#endif
