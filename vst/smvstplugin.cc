/*
 *  VST Plugin is under GPL 2 or later, since it is based on Vestige (GPL) and amsynth_vst (GPL)
 *
 *  Copyright (c) 2008-2015 Nick Dowell
 *  Copyright (c) 2016 Stefan Westerfeld
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "vestige/aeffectx.h"
#include "smutils.hh"
#include "smmorphplan.hh"
#include "smmorphplansynth.hh"
#include "smmidisynth.hh"
#include "smmain.hh"
#include "smvstui.hh"
#include "smvstplugin.hh"
#include "smmorphoutputmodule.hh"

// from http://www.asseca.org/vst-24-specs/index.html
#define effGetParamLabel        6
#define effGetParamDisplay      7
#define effGetChunk             23
#define effSetChunk             24
#define effCanBeAutomated       26
#define effGetOutputProperties  34
#define effVendorSpecific       50
#define effGetTailSize          52
#define effGetMidiKeyName       66
#define effBeginLoadBank        75
#define effFlagsProgramChunks   (1 << 5)

#define DEBUG 0

using namespace SpectMorph;

using std::string;

void
VstPlugin::preinit_plan (MorphPlanPtr plan)
{
  // this might take a while, and cannot be used in RT callback
  MorphPlanSynth mp_synth (mix_freq);
  MorphPlanVoice *mp_voice = mp_synth.add_voice();
  mp_synth.update_plan (plan);

  MorphOutputModule *om = mp_voice->output();
  if (om)
    {
      om->retrigger (0, 440, 1);
      float s;
      float *values[1] = { &s };
      om->process (1, values, 1);
    }
}

VstPlugin::VstPlugin (audioMasterCallback master, AEffect *aeffect) :
  audioMaster (master),
  aeffect (aeffect),
  plan (new MorphPlan()),
  midi_synth (nullptr)
{
  audioMaster = master;

  string filename = sm_get_default_plan();

  GenericIn *in = StdioIn::open (filename);
  if (in)
    {
      plan->load (in);
      delete in;
    }
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", filename.c_str());
      // in this case we fail gracefully and start with an empty plan
    }
  ui = new VstUI (plan->clone(), this);

  parameters.push_back (Parameter ("Control #1", 0, -1, 1));
  parameters.push_back (Parameter ("Control #2", 0, -1, 1));

  set_volume (-6); // default volume
  m_voices_active = false;

  // initialize mix_freq with something, so that the plugin doesn't crash if the host never calls SetSampleRate
  set_mix_freq (48000);
}

VstPlugin::~VstPlugin()
{
  delete ui;
  ui = nullptr;

  if (midi_synth)
    {
      delete midi_synth;
      midi_synth = nullptr;
    }
}

void
VstPlugin::change_plan (MorphPlanPtr plan)
{
  preinit_plan (plan);

  std::lock_guard<std::mutex> locker (m_new_plan_mutex);
  m_new_plan = plan;
}

void
VstPlugin::set_volume (double new_volume)
{
  std::lock_guard<std::mutex> locker (m_new_plan_mutex);
  m_volume = new_volume;
}

double
VstPlugin::volume()
{
  std::lock_guard<std::mutex> locker (m_new_plan_mutex);
  return m_volume;
}

bool
VstPlugin::voices_active()
{
  std::lock_guard<std::mutex> locker (m_new_plan_mutex);
  return m_voices_active;
}

void
VstPlugin::get_parameter_name (Param param, char *out, size_t len) const
{
  if (param >= 0 && param < parameters.size())
    strncpy (out, parameters[param].name.c_str(), len);
}

void
VstPlugin::get_parameter_label (Param param, char *out, size_t len) const
{
  if (param >= 0 && param < parameters.size())
    strncpy (out, parameters[param].label.c_str(), len);
}

void
VstPlugin::get_parameter_display (Param param, char *out, size_t len) const
{
  if (param >= 0 && param < parameters.size())
    strncpy (out, string_printf ("%.5f", parameters[param].value).c_str(), len);
}

void
VstPlugin::set_parameter_scale (Param param, float value)
{
  if (param >= 0 && param < parameters.size())
    parameters[param].value = parameters[param].min_value + (parameters[param].max_value - parameters[param].min_value) * value;
}

float
VstPlugin::get_parameter_scale (Param param) const
{
  if (param >= 0 && param < parameters.size())
    return (parameters[param].value - parameters[param].min_value) / (parameters[param].max_value - parameters[param].min_value);

  return 0;
}

float
VstPlugin::get_parameter_value (Param param) const
{
  if (param >= 0 && param < parameters.size())
    return parameters[param].value;

  return 0;
}

void
VstPlugin::set_parameter_value (Param param, float value)
{
  if (param >= 0 && param < parameters.size())
    parameters[param].value = value;
}

void
VstPlugin::set_mix_freq (double new_mix_freq)
{
  /* this should only be called by the host if the plugin is suspended, so
   * we can alter variables that are used by process|processReplacing in the real time thread
   */
  if (midi_synth)
    delete midi_synth;

  mix_freq = new_mix_freq;

  midi_synth = new MidiSynth (mix_freq, 64);
  midi_synth->update_plan (plan);
}


static char hostProductString[64] = "";

static intptr_t dispatcher(AEffect *effect, int opcode, int index, intptr_t val, void *ptr, float f)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;

  switch (opcode) {
    case effOpen:
      return 0;

    case effClose:
      delete plugin;
      memset(effect, 0, sizeof(AEffect));
      free(effect);
      return 0;

    case effSetProgram:
    case effGetProgram:
    case effGetProgramName:
      return 0;

    case effGetParamLabel:
      plugin->get_parameter_label ((VstPlugin::Param)index, (char *)ptr, 32);
      return 0;

    case effGetParamDisplay:
      plugin->get_parameter_display ((VstPlugin::Param)index, (char *)ptr, 32);
      return 0;

    case effGetParamName:
      plugin->get_parameter_name ((VstPlugin::Param)index, (char *)ptr, 32);
      return 0;

    case effSetSampleRate:
      plugin->set_mix_freq (f);
      return 0;

    case effSetBlockSize:
    case effMainsChanged:
      return 0;

    case effEditGetRect:
      plugin->ui->getRect ((ERect **) ptr);
      return 1;

    case effEditOpen:
      plugin->ui->open((PuglNativeWindow)(uintptr_t)ptr);
      return 1;

    case effEditClose:
      plugin->ui->close();
      return 0;

    case effEditIdle:
      plugin->ui->idle();
      return 0;

    case effGetChunk:
      {
        int result = plugin->ui->save_state((char **)ptr);
        VST_DEBUG ("get chunk returned: %s\n", *(char **)ptr);
        return result;
      }

    case effSetChunk:
      VST_DEBUG ("set chunk: %s\n", (char *)ptr);
      plugin->ui->load_state((char *)ptr);
      return 0;

    case effProcessEvents:
      {
        VstEvents *events = (VstEvents *)ptr;

        for (int32_t i = 0; i < events->numEvents; i++)
          {
            VstMidiEvent *event = (VstMidiEvent *)events->events[i];
            if (event->type != kVstMidiType)
              continue;

            plugin->midi_synth->add_midi_event (event->deltaFrames, reinterpret_cast <unsigned char *> (&event->midiData[0]));
          }
        return 1;
      }

    case effCanBeAutomated:
    case effGetOutputProperties:
      return 0;

    case effGetPlugCategory:
      return kPlugCategSynth;
    case effGetEffectName:
      strcpy((char *)ptr, "SpectMorph");
      return 1;
    case effGetVendorString:
      strcpy((char *)ptr, "Stefan Westerfeld");
      return 1;
    case effGetProductString:
      strcpy((char *)ptr, "SpectMorph");
      return 1;
    case effGetVendorVersion:
       return 0;
    case effCanDo:
      if (strcmp("receiveVstMidiEvent", (char *)ptr) == 0 || strcmp("MPE", (char *)ptr) == 0) return 1;
      if (strcmp("midiKeyBasedInstrumentControl", (char *)ptr) == 0 ||
              strcmp("receiveVstSysexEvent", (char *)ptr) == 0 ||
              strcmp("midiSingleNoteTuningChange", (char *)ptr) == 0 ||
              strcmp("sendVstMidiEvent", (char *)ptr) == 0 ||
              false) return 0;
      VST_DEBUG ("unhandled canDo: %s\n", (char *)ptr);
      return 0;

    case effGetTailSize:
    case effIdle:
    case effGetParameterProperties:
      return 0;

    case effGetVstVersion:
      return 2400;

    case effGetMidiKeyName:
    case effStartProcess:
    case effStopProcess:
    case effBeginSetProgram:
    case effEndSetProgram:
    case effBeginLoadBank:
      return 0;

    default:
      VST_DEBUG ("[smvstplugin] unhandled VST opcode: %d\n", opcode);
      return 0;
  }
}

static void
processReplacing (AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;

  // update plan with new parameters / new modules if necessary
  if (plugin->m_new_plan_mutex.try_lock())
    {
      if (plugin->m_new_plan)
        {
          plugin->midi_synth->update_plan (plugin->m_new_plan);
          plugin->m_new_plan = NULL;
        }
      plugin->rt_volume = plugin->m_volume;
      plugin->m_voices_active = plugin->midi_synth->active_voice_count() > 0;
      plugin->m_new_plan_mutex.unlock();
    }
  plugin->midi_synth->set_control_input (0, plugin->parameters[VstPlugin::PARAM_CONTROL_1].value);
  plugin->midi_synth->set_control_input (1, plugin->parameters[VstPlugin::PARAM_CONTROL_2].value);
  plugin->midi_synth->process (outputs[0], numSampleFrames);

  // apply replay volume
  const float volume_factor = db_to_factor (plugin->rt_volume);
  for (int i = 0; i < numSampleFrames; i++)
    outputs[0][i] *= volume_factor;

  std::copy (outputs[0], outputs[0] + numSampleFrames, outputs[1]);
}

static void
process (AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
  // this is not as efficient as it could be
  // however, most likely the actual morphing is a lot more expensive

  float  tmp_output_l[numSampleFrames];
  float  tmp_output_r[numSampleFrames];
  float *tmp_outputs[2] = { tmp_output_l, tmp_output_r };

  processReplacing (effect, inputs, tmp_outputs, numSampleFrames);

  for (int i = 0; i < numSampleFrames; i++)
    {
      outputs[0][i] += tmp_output_l[i];
      outputs[1][i] += tmp_output_r[i];
    }
}

static void setParameter(AEffect *effect, int i, float f)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;

  plugin->set_parameter_scale ((VstPlugin::Param) i, f);
}

static float getParameter(AEffect *effect, int i)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;

  return plugin->get_parameter_scale((VstPlugin::Param) i);
}

#ifdef SM_OS_WINDOWS
extern "C" AEffect *VSTPluginMain (audioMasterCallback audioMaster) __declspec(dllexport);
#endif

extern "C" AEffect *VSTPluginMain (audioMasterCallback audioMaster)
{
  if (DEBUG)
    {
      Debug::set_filename ("smvstplugin.log");
      Debug::enable ("vst");
      Debug::enable ("global");
    }

  VST_DEBUG ("VSTPluginMain called\n");
  if (audioMaster)
    {
      audioMaster (NULL, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
    }

  if (!sm_init_done())
    sm_init_plugin();

#ifdef SM_OS_WINDOWS
  // FIXME: use better strategy to find plugin data dir
  sm_set_pkg_data_dir ("c:/spectmorph");
#endif

  AEffect *effect = (AEffect *)calloc(1, sizeof(AEffect));
  effect->magic = kEffectMagic;
  effect->dispatcher = dispatcher;
  effect->process = process;
  effect->setParameter = setParameter;
  effect->getParameter = getParameter;
  effect->numPrograms = 0;
  effect->numParams = VstPlugin::PARAM_COUNT;
  effect->numInputs = 0;
  effect->numOutputs = 2;
  effect->flags = effFlagsCanReplacing | effFlagsIsSynth | effFlagsProgramChunks | effFlagsHasEditor;

  // Do no use the ->user pointer because ardour clobbers it
  effect->ptr3 = new VstPlugin (audioMaster, effect);
  effect->uniqueID = CCONST ('s', 'm', 'r', 'p');
  effect->processReplacing = processReplacing;

  VST_DEBUG ("VSTPluginMain done => return %p\n", effect);
  return effect;
}

#if 0
// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
#endif
