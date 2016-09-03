// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_VST_PLUGIN_HH
#define SPECTMORPH_VST_PLUGIN_HH

#include "vestige/aeffectx.h"
#include "smmorphplansynth.hh"
#include "smmidisynth.hh"

namespace SpectMorph
{

struct VstPlugin
{
  enum Param
  {
    PARAM_CONTROL_1 = 0,
    PARAM_CONTROL_2 = 1,
    PARAM_VOLUME    = 2,
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

  void  change_plan (MorphPlanPtr ptr);

  audioMasterCallback audioMaster;
  AEffect*            aeffect;

  MorphPlanPtr        plan;
  MorphPlanSynth      morph_plan_synth;
  MidiSynth           midi_synth;
  VstUI              *ui;

  QMutex                        m_new_plan_mutex;
  MorphPlanPtr                  m_new_plan;
};

}

#endif
