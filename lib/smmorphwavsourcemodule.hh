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

class VoiceSource {
  double m_ratio = 0;
  int max_partials = 0;
  // FIXME: default
  MorphWavSource::FormantCorrection mode;
  float fuzzy_resynth = 0;
  double fuzzy_frac = 0;
  std::vector<float> detune_factors;
  std::vector<float> next_detune_factors;
  void gen_detune_factors (std::vector<float>& factors);
  Random detune_random;
public:
  VoiceSource()
  {
    detune_factors.reserve (400);
    next_detune_factors.reserve (400);
  }
  void
  set_mode (MorphWavSource::FormantCorrection new_mode)
  {
    mode = new_mode;
  }
  void
  set_fuzzy_resynth (float new_fuzzy_resynth)
  {
    /* non-linear mapping from percent to cent: allow better control for small cent values */
    double f = new_fuzzy_resynth * 0.01;
    fuzzy_resynth = (f + 2 * f * f) * 16 / 3;
  }
  void
  set_max_partials (int new_max_partials)
  {
    max_partials = new_max_partials;
  }
  void advance (double time_ms);
  void retrigger();
  void set_ratio (double ratio);
  void process_block (const AudioBlock& in_block, RTAudioBlock& out_block);
};

class MorphWavSourceModule : public MorphOperatorModule
{
  const MorphWavSource::Config *cfg = nullptr;

  class InstrumentSource : public LiveDecoderSource
  {
    VoiceSource             voice_source;
    double                  last_time_ms = 0;
    Audio                  *active_audio = nullptr;
    int                     object_id = 0;
    Project                *project = nullptr;
  public:
    MorphWavSourceModule   *module = nullptr;

    void
    set_portamento_freq (float freq) override
    {
      if (active_audio)
        voice_source.set_ratio (freq / active_audio->fundamental_freq);
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
  ~MorphWavSourceModule();

  void set_config (const MorphOperatorConfig *op_cfg);
  LiveDecoderSource *source();
};

}

#endif
