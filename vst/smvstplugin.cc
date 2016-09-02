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

#include <QMutex>
#include <QApplication>

// from http://www.asseca.org/vst-24-specs/index.html
#define effGetParamLabel        6
#define effGetParamDisplay      7
#define effGetChunk             23
#define effSetChunk             24
#define effCanBeAutomated       26
#define effGetOutputProperties  34
#define effGetTailSize          52
#define effGetMidiKeyName       66
#define effBeginLoadBank        75
#define effFlagsProgramChunks   (1 << 5)

#define FIXME_MIX_FREQ 48000
#define DEBUG 1

using namespace SpectMorph;

using std::string;

static FILE *debug_file = NULL;
QMutex       debug_mutex;

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
      QMutexLocker locker (&debug_mutex);

      if (!debug_file)
        debug_file = fopen ("/tmp/smvstplugin.log", "w");

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", string_vprintf (fmt, ap).c_str());
      va_end (ap);
      fflush (debug_file);
    }
}

static bool
is_note_on (const VstMidiEvent *event)
{
  if ((event->midiData[0] & 0xf0) == 0x90)
    {
      if (event->midiData[2] != 0) /* note on with velocity 0 => note off */
        return true;
    }
  return false;
}

static bool
is_note_off (const VstMidiEvent *event)
{
  if ((event->midiData[0] & 0xf0) == 0x90)
    {
      if (event->midiData[2] == 0) /* note on with velocity 0 => note off */
        return true;
    }
  else if ((event->midiData[0] & 0xf0) == 0x80)
    {
      return true;
    }
  return false;
}

void
preinit_plan (MorphPlanPtr plan)
{
  // this might take a while, and cannot be used in RT callback
  MorphPlanSynth mp_synth (FIXME_MIX_FREQ); // FIXME
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

void
VstPlugin::change_plan (MorphPlanPtr plan)
{
  preinit_plan (plan);

  QMutexLocker locker (&m_new_plan_mutex);
  m_new_plan = plan;
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

#if 0 // FIXME
		case effGetParamLabel:
			// FIXME plugin->synthesizer->getParameterLabel((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamDisplay:
			// FIXME plugin->synthesizer->getParameterDisplay((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamName:
			// FIXME plugin->synthesizer->getParameterName((Param)index, (char *)ptr, 32);
			return 0;

#endif
		case effSetSampleRate:
			debug ("fake set sample rate %f\n", f); //// FIXME plugin->synthesizer->setSampleRate(f);
			return 0;

		case effSetBlockSize:
		case effMainsChanged:
			return 0;

    case effEditGetRect:
      plugin->ui->getRect ((ERect **) ptr);
      return 1;

    case effEditOpen:
      plugin->ui->open((WId)(uintptr_t)ptr);
      return 1;

    case effEditClose:
      plugin->ui->close();
      return 0;

    case effEditIdle:
      plugin->ui->idle();
      return 0;

#if 0 // FIXME
		case effGetChunk:
			// FIXME return plugin->synthesizer->saveState((char **)ptr);

		case effSetChunk:
			// FIXME plugin->synthesizer->loadState((char *)ptr);
			return 0;
#endif

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

			for (int32_t i=0; i<events->numEvents; i++)
                          {
                            VstMidiEvent *event = (VstMidiEvent *)events->events[i];
                            if (event->type != kVstMidiType)
                              continue;

                            debug ("EV: %x %d %d\n", event->midiData[0], event->midiData[1], event->midiData[2]);
                            if (is_note_on (event))
                              {
                                plugin->midi_synth.process_note_on (event->midiData[1], event->midiData[2]);
                              }
                            else if (is_note_off (event))
                              {
                                plugin->midi_synth.process_note_off (event->midiData[1]);
                              }
			  }

			return 1;
#if 0 // FIXME
			VstEvents *events = (VstEvents *)ptr;

			assert(plugin->midiEvents.empty());

			memset(plugin->midiBuffer, 0, 4096);
			unsigned char *buffer = plugin->midiBuffer;
			
			for (int32_t i=0; i<events->numEvents; i++) {
				VstMidiEvent *event = (VstMidiEvent *)events->events[i];
				if (event->type != kVstMidiType) {
					continue;
				}

				memcpy(buffer, event->midiData, 4);

				amsynth_midi_event_t midi_event;
				memset(&midi_event, 0, sizeof(midi_event));
				midi_event.offset_frames = event->deltaFrames;
				midi_event.buffer = buffer;
				midi_event.length = 4;
				plugin->midiEvents.push_back(midi_event);

				buffer += event->byteSize;

				assert(buffer < plugin->midiBuffer + 4096);
#endif
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
#if 0
		case effGetProductString:
			strcpy((char *)ptr, "amsynth");
			return 1;
#endif
		case effGetVendorVersion:
			return 0;

		case effCanDo:
			if (strcmp("receiveVstMidiEvent", (char *)ptr) == 0 ||
				false) return 1;
			if (strcmp("midiKeyBasedInstrumentControl", (char *)ptr) == 0 ||
				strcmp("midiSingleNoteTuningChange", (char *)ptr) == 0 ||
				strcmp("receiveVstSysexEvent", (char *)ptr) == 0 ||
				strcmp("sendVstMidiEvent", (char *)ptr) == 0 ||
				false) return 0;
			debug("unhandled canDo: %s\n", (char *)ptr);
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
			debug ("[smvstplugin] unhandled VST opcode: %d\n", opcode);
			return 0;
	}
}

static void process(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;
  debug ("!missing process\n");
#if 0 // FIXME
  std::vector<amsynth_midi_cc_t> midi_out;
  plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
  plugin->midiEvents.clear();
#endif
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;

  // update plan with new parameters / new modules if necessary
  if (plugin->m_new_plan_mutex.tryLock())
    {
      if (plugin->m_new_plan)
        {
          plugin->morph_plan_synth.update_plan (plugin->m_new_plan);
          plugin->m_new_plan = NULL;
        }
      plugin->m_new_plan_mutex.unlock();
    }

  plugin->midi_synth.process_audio (outputs[0], numSampleFrames);
  std::copy_n (outputs[0], numSampleFrames, outputs[1]);
}

static void setParameter(AEffect *effect, int i, float f)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;
  debug ("!missing set parameter\n");

  // FIXME plugin->ynthesizer->setNormalizedParameterValue((Param) i, f);
}

static float getParameter(AEffect *effect, int i)
{
  VstPlugin *plugin = (VstPlugin *)effect->ptr3;
  debug ("!missing get parameter\n");

  // FIXME return plugin->synthesizer->getNormalizedParameterValue((Param) i);
}

extern "C" AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
  debug ("VSTPluginMain called\n");
  if (audioMaster)
    {
      audioMaster (NULL, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
    }

  if (!sm_init_done())
    sm_init_plugin();

  if (qApp)
    {
      debug ("... (have qapp) ...\n");
    }
  else
    {
      printf ("...  (creating qapp) ...\n");
      static int argc = 0;
      new QApplication(argc, NULL, true);
    }

  AEffect *effect = (AEffect *)calloc(1, sizeof(AEffect));
  effect->magic = kEffectMagic;
  effect->dispatcher = dispatcher;
  effect->process = process;
  effect->setParameter = setParameter;
  effect->getParameter = getParameter;
  effect->numPrograms = 0;
  effect->numParams = 0; // FIXME kAmsynthParameterCount;
  effect->numInputs = 0;
  effect->numOutputs = 2;
  effect->flags = effFlagsCanReplacing | effFlagsIsSynth | effFlagsProgramChunks | effFlagsHasEditor;

  // Do no use the ->user pointer because ardour clobbers it
  effect->ptr3 = new VstPlugin(audioMaster);
  effect->uniqueID = CCONST ('s', 'm', 'r', 'p');
  effect->processReplacing = processReplacing;

  debug ("VSTPluginMain done => return %p\n", effect);
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
