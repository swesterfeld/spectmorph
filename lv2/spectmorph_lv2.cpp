// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"

#define SPECTMORPH_URI "http://spectmorph.org/plugins/spectmorph"

using namespace SpectMorph;
using std::vector;
using std::max;

enum PortIndex {
  SPECTMORPH_MIDI_IN  = 0,
  SPECTMORPH_GAIN     = 1,
  SPECTMORPH_INPUT    = 2,
  SPECTMORPH_OUTPUT   = 3
};

class Voice
{
public:
  enum State {
    STATE_IDLE,
    STATE_ON,
    STATE_RELEASE
  };
  MorphPlanVoice *mp_voice;

  State        state;
  bool         pedal;
  int          midi_note;
  double       env;
  double       velocity;

  Voice() :
    mp_voice (NULL),
    state (STATE_IDLE),
    pedal (false)
  {
  }
  ~Voice()
  {
    mp_voice = NULL;
  }
};

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

class MidiHandler
{
  vector<Voice> voices;
  double        mix_freq;
public:
  MidiHandler (MorphPlanSynth& synth, double mix_freq, size_t n_voices);

  void process_note_on (int midi_note, int midi_velocity);
  void process_note_off (int midi_note);
  void process_audio (float *output, size_t n_values);
};

MidiHandler::MidiHandler (MorphPlanSynth& synth, double mix_freq, size_t n_voices)
  : mix_freq (mix_freq)
{
  voices.clear();
  voices.resize (n_voices);

  for (auto& v : voices)
    {
      v.mp_voice = synth.add_voice();
    }
}

void
MidiHandler::process_note_on (int midi_note, int midi_velocity)
{
  for (auto& v : voices)
    {
      if (v.state == Voice::STATE_IDLE)
        {
          MorphOutputModule *output = v.mp_voice->output();
          output->retrigger (0 /* channel */, freq_from_note (midi_note), midi_velocity);

          v.state     = Voice::STATE_ON;
          v.midi_note = midi_note;
          v.velocity  = midi_velocity;

          return;
        }
    }
}

void
MidiHandler::process_note_off (int midi_note)
{
  for (auto& v : voices)
    {
      if (v.state == Voice::STATE_ON && v.midi_note == midi_note)
        {
          /* FIXME: hande pedal */
          v.state = Voice::STATE_RELEASE;
          v.env = 1.0;
        }
    }
}

void
MidiHandler::process_audio (float *output, size_t n_values)
{
  zero_float_block (n_values, output);

  float samples[n_values];
  float *values[1] = { samples };

  for (auto& v : voices)
    {
      if (v.state == Voice::STATE_ON)
        {
          MorphOutputModule *output_module = v.mp_voice->output();

          output_module->process (n_values, values, 1);
          for (size_t i = 0; i < n_values; i++)
            output[i] += samples[i];
        }
      else if (v.state == Voice::STATE_RELEASE)
        {
          const float release_ms = 150; /* FIXME: this should be set by the user */

          double v_decrement = (1000.0 / mix_freq) / release_ms;
          size_t envelope_len = qBound<int> (0, sm_round_positive (v.env / v_decrement), n_values);

          if (envelope_len < n_values)
            {
              /* envelope reached zero -> voice can be reused later */
              v.state = Voice::STATE_IDLE;
              v.pedal = false;
            }
          MorphOutputModule *output_module = v.mp_voice->output();

          output_module->process (envelope_len, values, 1);
          for (size_t i = 0; i < envelope_len; i++)
            {
              v.env -= v_decrement;
              output[i] += samples[i] * v.env;
            }
        }
    }
}

class SpectMorphLV2
{
public:
  // Port buffers
  const LV2_Atom_Sequence* midi_in;
  const float* gain;
  const float* input;
  float*       output;

  // Features
  LV2_URID_Map* map;

  struct {
    LV2_URID midi_MidiEvent;
  } uris;

  SpectMorphLV2 (double mix_freq);

  // SpectMorph stuff
  double          mix_freq;
  MorphPlanSynth  morph_plan_synth;
  MorphPlanPtr    plan;
  MidiHandler     midi_handler;
};

SpectMorphLV2::SpectMorphLV2 (double mix_freq) :
  mix_freq (mix_freq),
  morph_plan_synth (mix_freq),
  plan (new MorphPlan()),
  midi_handler (morph_plan_synth, mix_freq, 64)
{
  std::string filename = "/home/stefan/lv2.smplan";
  GenericIn *in = StdioIn::open (filename);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", filename.c_str());
      exit (1);
    }
  plan->load (in);
  delete in;

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());
  morph_plan_synth.update_plan (plan);
}

static LV2_Handle
instantiate (const LV2_Descriptor*     descriptor,
             double                    rate,
             const char*               bundle_path,
             const LV2_Feature* const* features)
{
  if (!sm_init_done())
    sm_init_plugin();

  SpectMorphLV2 *self = new SpectMorphLV2 (rate);

  LV2_URID_Map* map = NULL;
  for (int i = 0; features[i]; i++)
    {
      if (!strcmp (features[i]->URI, LV2_URID__map))
        {
          map = (LV2_URID_Map*)features[i]->data;
          break;
        }
    }
  if (!map)
    {
      delete self;
      return NULL; // host bug, we need this feature
    }

  self->map = map;
  self->uris.midi_MidiEvent = map->map (map->handle, LV2_MIDI__MidiEvent);

  return (LV2_Handle)self;
}

static void
connect_port (LV2_Handle instance,
              uint32_t   port,
              void*      data)
{
  SpectMorphLV2* self = (SpectMorphLV2*)instance;

  switch ((PortIndex)port)
    {
      case SPECTMORPH_MIDI_IN:    self->midi_in = (const LV2_Atom_Sequence*)data;
                                  break;
      case SPECTMORPH_GAIN:       self->gain = (const float*)data;
                                  break;
      case SPECTMORPH_INPUT:      self->input = (const float*)data;
                                  break;
      case SPECTMORPH_OUTPUT:     self->output = (float*)data;
                                  break;
    }
}

static void
activate (LV2_Handle instance)
{
}

static void
run (LV2_Handle instance, uint32_t n_samples)
{
  SpectMorphLV2* self = (SpectMorphLV2*)instance;

  const float        gain   = *(self->gain);
  const float* const input  = self->input;
  float* const       output = self->output;

  uint32_t  offset = 0;

  LV2_ATOM_SEQUENCE_FOREACH (self->midi_in, ev)
    {
      if (ev->body.type == self->uris.midi_MidiEvent)
        {
          const uint8_t* const msg = (const uint8_t*)(ev + 1);
          switch (lv2_midi_message_type (msg))
            {
              case LV2_MIDI_MSG_NOTE_ON:
                self->midi_handler.process_note_on (msg[1], msg[2]);
                break;
              case LV2_MIDI_MSG_NOTE_OFF:
                self->midi_handler.process_note_off (msg[1]);
                break;
              //case LV2_MIDI_MSG_PGM_CHANGE:
              default: break;
            }

          //write_output(self, offset, ev->time.frames - offset);
          offset = (uint32_t)ev->time.frames;
        }
    }
  // write_output(self, offset, sample_count - offset);
  self->midi_handler.process_audio (output, n_samples);
  self->morph_plan_synth.update_shared_state (n_samples / self->mix_freq * 1000);
}

static void
deactivate (LV2_Handle instance)
{
}

static void
cleanup (LV2_Handle instance)
{
  delete static_cast <SpectMorphLV2 *> (instance);
}

static const void*
extension_data (const char* uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {
  SPECTMORPH_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor (uint32_t index)
{
  switch (index)
    {
      case 0:  return &descriptor;
      default: return NULL;
    }
}
