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
#include <glib/gstdio.h>

#include "vestige/aeffectx.h"
#if 0 // WIN32
#include "smutils.hh"
#include "smmorphplan.hh"
#include "smmorphplansynth.hh"
#include "smmidisynth.hh"
#include "smmain.hh"

#include <QMutex>
#include <QPushButton>
#include <QApplication>
#else
#include <glib.h>
#endif
#include <algorithm>
#include <string>

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

#define VST_PLUGIN 1
#include "smuitest.cc"

#define DEBUG 1

#if 0
using namespace SpectMorph;
#endif

static FILE *debug_file = NULL;
#if 0
QMutex       debug_mutex;
#endif

using std::string;

typedef uintptr_t WId;

string
get_plugin_dir()
{
#if WIN32
  return "C:/msys64/home/stefan/src/sandbox/smvstp";
#else
  string home = g_get_home_dir();
  return home + "/src/sandbox/smvstp";
#endif
}

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
#if 0
      QMutexLocker locker (&debug_mutex);
#endif

      if (!debug_file)
	{
	  string debug_file_name = get_plugin_dir() + "/vstplugin.log";
	  debug_file = fopen (debug_file_name.c_str(), "w");
	}

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", g_strdup_vprintf (fmt, ap)); // FIXME: leak
      va_end (ap);
      fflush (debug_file);
    }
}

#if 0
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
#endif

struct ERect
{
	short top;
	short left;
	short bottom;
	short right;
};

static char hostProductString[64] = "";

class VstUI
{
  ERect rectangle;
  MainWindow *main_window;
public:
  VstUI()
  {
    rectangle.top = 0;
    rectangle.left = 0;
    rectangle.bottom = 512;
    rectangle.right  = 512;
  }
  bool
  open (WId win_id)
  {
    debug ("... create MainWindow\n");
    main_window = new MainWindow (512, 512, win_id);
    debug ("... done\n");

    main_window->show();

    return true;
  }
  bool
  getRect (ERect** rect)
  {
    *rect = &rectangle;
    debug ("getRect -> %d %d %d %d\n", rectangle.top, rectangle.left, rectangle.bottom, rectangle.right);

    return true;
  }
  void
  close()
  {
    delete main_window;
  }
  void
  idle()
  {
    main_window->process_events();
  }
};

struct Plugin
{
  Plugin(audioMasterCallback master) :
#if 0 // WIN32
    plan (new MorphPlan()),
    morph_plan_synth (48000), // FIXME
    midi_synth (morph_plan_synth, 48000, 64), // FIXME
#endif
    ui (new VstUI)
  {
    audioMaster = master;

#if 0 // WIN32
    std::string filename = "/home/stefan/lv2.smplan";
    GenericIn *in = StdioIn::open (filename);
    if (!in)
      {
        g_printerr ("Error opening '%s'.\n", filename.c_str());
        exit (1);
      }
    plan->load (in);
    delete in;
#endif
#if 0
    morph_plan_synth.update_plan (plan);
#endif
  }

  ~Plugin()
  {
    delete ui;
    ui = nullptr;
  }

  audioMasterCallback audioMaster;
#if 0 // WIN32
  MorphPlanPtr        plan;
  MorphPlanSynth      morph_plan_synth;
  MidiSynth           midi_synth;
#endif
  VstUI              *ui;
};

static intptr_t dispatcher(AEffect *effect, int opcode, int index, intptr_t val, void *ptr, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

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

#if 0 // WIN32
			    debug ("EV: %x %d %d\n", event->midiData[0], event->midiData[1], event->midiData[2]);
                            if (is_note_on (event))
                              {
                                plugin->midi_synth.process_note_on (event->midiData[1], event->midiData[2]);
                              }
                            else if (is_note_off (event))
                              {
                                plugin->midi_synth.process_note_off (event->midiData[1]);
                              }
#endif
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
			strcpy((char *)ptr, "SpectMorph Test");
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
#if 0 // FIXME
  Plugin *plugin = (Plugin *)effect->ptr3;
  debug ("!missing process\n");
  std::vector<amsynth_midi_cc_t> midi_out;
  plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
  plugin->midiEvents.clear();
#endif
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
#if 0 // WIN32
  Plugin *plugin = (Plugin *)effect->ptr3;

  plugin->midi_synth.process_audio (outputs[0], numSampleFrames);
  std::copy_n (outputs[0], numSampleFrames, outputs[1]);
#else
  std::fill_n (outputs[0], numSampleFrames, 0.0);
  std::fill_n (outputs[1], numSampleFrames, 0.0);
#endif
}

static void setParameter(AEffect *effect, int i, float f)
{
#if 0
  Plugin *plugin = (Plugin *)effect->ptr3;
  debug ("!missing set parameter\n");

  // FIXME plugin->ynthesizer->setNormalizedParameterValue((Param) i, f);
#endif
}

static float getParameter(AEffect *effect, int i)
{
#if 0
  Plugin *plugin = (Plugin *)effect->ptr3;
  debug ("!missing get parameter\n");

  // FIXME return plugin->synthesizer->getNormalizedParameterValue((Param) i);
#else
  return 0;
#endif
}

extern "C" {

#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
	#define VST_EXPORT	__attribute__ ((visibility ("default")))
#else
	#define VST_EXPORT
#endif

VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster) 
#if WIN32
  __declspec(dllexport)
#endif
  ;

VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
  debug ("VSTPluginMain called\n");
  if (audioMaster)
    {
      audioMaster (NULL, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
    }

  if (!sm_init_done())
    sm_init_plugin();

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
  effect->ptr3 = new Plugin(audioMaster);
  effect->uniqueID = CCONST ('s', 'm', 'T', 'p'); // T => test
  effect->processReplacing = processReplacing;

  debug ("VSTPluginMain done => return %p\n", effect);
  return effect;
}

// support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif WIN32
VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif BEOS
VST_EXPORT AEffect* main_plugin (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#endif

} // extern "C"

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
void* hInstance;

extern "C" {
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
} // extern "C"
#endif
#if 00
extern "C" AEffect * MAIN(audioMasterCallback audioMaster) __declspec(dllexport);

extern "C" __attribute__ ((visibility("default"))) AEffect * MAIN(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
#endif
#if 0
// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
#endif
