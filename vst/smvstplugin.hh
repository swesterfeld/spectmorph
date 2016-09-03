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

    Parameter (const char *name, float min_value = 0, float max_value = 1, std::string label = "") :
      name (name),
      value (0),
      min_value (min_value),
      max_value (max_value),
      label (label)
    {
    }
  };
  std::vector<Parameter> parameters;

  void get_parameter_name (Param param, char *out, size_t len) const;
  void get_parameter_label (Param param, char *out, size_t len) const;
  void get_parameter_display (Param param, char *out, size_t len) const;
  void set_parameter_scale (Param param, float value);

  VstPlugin (audioMasterCallback master, AEffect *aeffect) :
    audioMaster (master),
    aeffect (aeffect),
    plan (new MorphPlan()),
    morph_plan_synth (48000), // FIXME
    midi_synth (morph_plan_synth, 48000, 64), // FIXME
    ui (new VstUI ("/home/stefan/lv2.smplan", this))
  {
    audioMaster = master;

    std::string filename = "/home/stefan/lv2.smplan";
    GenericIn *in = StdioIn::open (filename);
    if (!in)
      {
        g_printerr ("Error opening '%s'.\n", filename.c_str());
        exit (1);
      }
    plan->load (in);
    delete in;

    morph_plan_synth.update_plan (plan);

    parameters.push_back (Parameter ("Control #1", -1, 1));
    parameters.push_back (Parameter ("Control #2", -1, 1));
    parameters.push_back (Parameter ("Volume", -48, 12, "dB"));
  }

  ~VstPlugin()
  {
    delete ui;
    ui = nullptr;
  }

  void change_plan (MorphPlanPtr ptr);

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
