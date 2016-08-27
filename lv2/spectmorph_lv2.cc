// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmidisynth.hh"
#include "smmain.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smlv2common.hh"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"

using namespace SpectMorph;
using std::string;
using std::vector;
using std::max;

enum PortIndex {
  SPECTMORPH_MIDI_IN  = 0,
  SPECTMORPH_GAIN     = 1,
  SPECTMORPH_INPUT    = 2,
  SPECTMORPH_OUTPUT   = 3,
  SPECTMORPH_NOTIFY   = 4
};

class SpectMorphLV2 : public LV2Common
{
public:
  // Port buffers
  const LV2_Atom_Sequence* midi_in;
  const float* gain;
  const float* input;
  float*       output;
  LV2_Atom_Sequence* notify_port;

  // Logger
  LV2_Log_Log*          log;
  LV2_Log_Logger        logger;

  LV2_Worker_Schedule*  schedule;

  LV2_Atom_Forge  forge;

  // Forge frame for notify port
  LV2_Atom_Forge_Frame notify_frame;

  SpectMorphLV2 (double mix_freq);

  // SpectMorph stuff
  double          mix_freq;
  MorphPlanSynth  morph_plan_synth;
  MorphPlanPtr    plan;
  MidiSynth       midi_synth;
  string          plan_str;
};

SpectMorphLV2::SpectMorphLV2 (double mix_freq) :
  mix_freq (mix_freq),
  morph_plan_synth (mix_freq),
  plan (new MorphPlan()),
  midi_synth (morph_plan_synth, mix_freq, 64)
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

  // only needed as long as we load a default plan
  vector<unsigned char> data;
  MemOut mo (&data);
  plan->save (&mo);
  plan_str = HexString::encode (data);

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
  self->schedule = NULL;
  for (int i = 0; features[i]; i++)
    {
      if (!strcmp (features[i]->URI, LV2_URID__map))
        {
          map = (LV2_URID_Map*)features[i]->data;
        }
      else if (!strcmp(features[i]->URI, LV2_LOG__log))
        {
          self->log = (LV2_Log_Log*) features[i]->data;
        }
      else if (!strcmp(features[i]->URI, LV2_WORKER__schedule))
        {
          self->schedule = (LV2_Worker_Schedule*)features[i]->data;
        }
    }
  if (!map)
    {
      delete self;
      return NULL; // host bug, we need this feature
    }
  else if (!self->schedule)
    {
      lv2_log_error (&self->logger, "Missing feature work:schedule\n");
      delete self;
      return NULL; // host bug, we need this feature
    }

  self->init_map (map);

  lv2_atom_forge_init (&self->forge, self->map);
  lv2_log_logger_init (&self->logger, self->map, self->log);

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
      case SPECTMORPH_NOTIFY:     self->notify_port = (LV2_Atom_Sequence*)data;
                                  break;
    }
}

static void
activate (LV2_Handle instance)
{
}

/**
   Do work in a non-realtime thread.

   This is called for every piece of work scheduled in the audio thread using
   self->schedule->schedule_work().  A reply can be sent back to the audio
   thread using the provided respond function.
*/
static LV2_Worker_Status
work (LV2_Handle                  instance,
      LV2_Worker_Respond_Function respond,
      LV2_Worker_Respond_Handle   handle,
      uint32_t                    size,
      const void*                 data)
{
  SpectMorphLV2* self = (SpectMorphLV2*)instance;

  const LV2_Atom* atom = (const LV2_Atom*)data;

  // Handle set message (change plan).
  const LV2_Atom_Object* obj = (const LV2_Atom_Object*)data;

  // Get file path from message
  const LV2_Atom* file_path = self->read_set_file (obj);
  if (!file_path)
    return LV2_WORKER_ERR_UNKNOWN;

  // Load sample.
  const char *plan = static_cast<const char *> (LV2_ATOM_BODY_CONST (file_path));
  printf ("PLUGIN:%s\n", plan);
#if 0
                if (sample) {
                        // Loaded sample, send it to run() to be applied.
                        respond(handle, sizeof(sample), &sample);
                }
#endif

  return LV2_WORKER_SUCCESS;
}

/**
   Handle a response from work() in the audio thread.

   When running normally, this will be called by the host after run().  When
   freewheeling, this will be called immediately at the point the work was
   scheduled.
*/
static LV2_Worker_Status
work_response(LV2_Handle  instance,
              uint32_t    size,
              const void* data)
{
#if 0
        Sampler* self = (Sampler*)instance;

        SampleMessage msg = { { sizeof(Sample*), self->uris.eg_freeSample },
                              self->sample };

        // Send a message to the worker to free the current sample
        self->schedule->schedule_work(self->schedule->handle, sizeof(msg), &msg);

        // Install the new sample
        self->sample = *(Sample*const*)data;

        // Send a notification that we're using a new sample.
        lv2_atom_forge_frame_time(&self->forge, self->frame_offset);
        write_set_file(&self->forge, &self->uris,
                       self->sample->path,
                       self->sample->path_len);
#endif
        return LV2_WORKER_SUCCESS;
}

static void
run (LV2_Handle instance, uint32_t n_samples)
{
  SpectMorphLV2* self = (SpectMorphLV2*)instance;

  const float        gain   = *(self->gain);
  const float* const input  = self->input;
  float* const       output = self->output;

  uint32_t  offset = 0;

  // Set up forge to write directly to notify output port.
  const uint32_t notify_capacity = self->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&self->forge,
                            (uint8_t*)self->notify_port,
                            notify_capacity);

  // Start a sequence in the notify output port.
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);


  LV2_ATOM_SEQUENCE_FOREACH (self->midi_in, ev)
    {
      if (ev->body.type == self->uris.midi_MidiEvent)
        {
          const uint8_t* const msg = (const uint8_t*)(ev + 1);
          switch (lv2_midi_message_type (msg))
            {
              case LV2_MIDI_MSG_NOTE_ON:
                self->midi_synth.process_note_on (msg[1], msg[2]);
                break;
              case LV2_MIDI_MSG_NOTE_OFF:
                self->midi_synth.process_note_off (msg[1]);
                break;
              case LV2_MIDI_MSG_CONTROLLER:
                self->midi_synth.process_midi_controller (msg[1], msg[2]);
                break;
              //case LV2_MIDI_MSG_PGM_CHANGE:
              default: break;
            }

          //write_output(self, offset, ev->time.frames - offset);
          offset = (uint32_t)ev->time.frames;
        }
      else if (lv2_atom_forge_is_object_type (&self->forge, ev->body.type))
        {
          const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

          if (obj->body.otype == self->uris.patch_Set)
            {
              // Get the property and value of the set message
              const LV2_Atom* property = NULL;
              const LV2_Atom* value    = NULL;
              lv2_atom_object_get  (obj,
                                    self->uris.patch_property, &property,
                                    self->uris.patch_value,    &value,
                                    0);
              if (!property)
                {
                  lv2_log_error (&self->logger, "patch:Set message with no property\n");
                  continue;
                }
              else if (property->type != self->uris.atom_URID)
                {
                  lv2_log_error (&self->logger, "patch:Set property is not a URID\n");
                  continue;
                }

              const uint32_t key = ((const LV2_Atom_URID*)property)->body;
              if (key == self->uris.spectmorph_plan)
                {
                  // Sample change, send it to the worker.
                  lv2_log_trace (&self->logger, "Queueing set message\n");
                  printf ("plugin: qset\n");

                  self->schedule->schedule_work (self->schedule->handle, lv2_atom_total_size (&ev->body), &ev->body);
                }
#if 0
 else if (key == uris->param_gain) {
                                        // Gain change
                                        if (value->type == uris->atom_Float) {
                                                self->gain = DB_CO(((LV2_Atom_Float*)value)->body);
                                        }
                                }
#endif
            }
          if (obj->body.otype == self->uris.patch_Get)
            {
              printf ("received get event\n");
              const char *state = self->plan_str.c_str();
              lv2_atom_forge_frame_time(&self->forge, offset);

              self->write_set_file (&self->forge, state, strlen (state));
            }
        }
    }
  // write_output(self, offset, sample_count - offset);
  self->midi_synth.process_audio (output, n_samples);
  self->morph_plan_synth.update_shared_state (n_samples / self->mix_freq * 1000);

  // apply post gain
  const float v = (gain > -90) ? bse_db_to_factor (gain) : 0;
  for (uint32_t i = 0; i < n_samples; i++)
    output[i] *= v;
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
  static const LV2_Worker_Interface worker = { work, work_response, NULL };

  if (!strcmp(uri, LV2_WORKER__interface))
    {
      return &worker;
    }
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
