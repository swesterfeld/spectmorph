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
