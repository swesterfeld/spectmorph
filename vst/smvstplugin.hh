// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_VST_PLUGIN_HH
#define SPECTMORPH_VST_PLUGIN_HH

#include "vestige/aeffectx.h"
#include "smmorphplansynth.hh"
#include "smmidisynth.hh"

#define VST_DEBUG(...) Debug::debug ("vst", __VA_ARGS__)

namespace SpectMorph
{

struct VstPlugin : public SynthInterface
{
  enum Param
  {
    PARAM_CONTROL_1 = 0,
    PARAM_CONTROL_2 = 1,
    PARAM_COUNT
  };

  struct Parameter
  {
    std::string name;
    float       value;
    float       min_value;
    float       max_value;
    std::string label;

    Parameter (const char *name, float default_value, float min_value, float max_value, std::string label = "") :
      name (name),
      value (default_value),
      min_value (min_value),
      max_value (max_value),
      label (label)
    {
    }
  };
  std::vector<Parameter> parameters;

  VstPlugin (audioMasterCallback master, AEffect *aeffect);
  ~VstPlugin();

  void  get_parameter_name (Param param, char *out, size_t len) const;
  void  get_parameter_label (Param param, char *out, size_t len) const;
  void  get_parameter_display (Param param, char *out, size_t len) const;

  float get_parameter_scale (Param param) const;
  void  set_parameter_scale (Param param, float value);

  float get_parameter_value (Param param) const;
  void  set_parameter_value (Param param, float value);

  void  set_volume (double new_volume);
  double volume();

  bool  voices_active();

  void  set_mix_freq (double mix_freq);
  void  preinit_plan (MorphPlanPtr plan);

  void  change_plan (MorphPlanPtr ptr);
  void  synth_inst_edit_update (bool active, const std::string& filename, bool orig_samples);

  audioMasterCallback audioMaster;
  AEffect*            aeffect;

  MorphPlanPtr        plan;
  MidiSynth          *midi_synth;
  VstUI              *ui;
  double              mix_freq;

  std::mutex          m_new_plan_mutex;
  MorphPlanPtr        m_new_plan;
  double              m_volume;
  bool                m_voices_active;
  double              rt_volume; // realtime thread only

  bool                m_have_inst_edit_update;
  InstEditUpdate      m_inst_edit_update;
};

}

#endif
