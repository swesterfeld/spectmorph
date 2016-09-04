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
#include "smutils.hh"
#include "smlv2common.hh"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

using namespace SpectMorph;
using std::string;
using std::vector;
using std::max;

#define DEBUG 1

static FILE *debug_file = NULL;
QMutex       debug_mutex;

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
      QMutexLocker locker (&debug_mutex);

      if (!debug_file)
        debug_file = fopen ("/tmp/smlv2plugin.log", "w");

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", string_vprintf (fmt, ap).c_str());
      va_end (ap);
      fflush (debug_file);
    }
}

enum PortIndex {
  SPECTMORPH_MIDI_IN  = 0,
  SPECTMORPH_GAIN     = 1,
  SPECTMORPH_OUTPUT   = 2,
  SPECTMORPH_NOTIFY   = 3
};

namespace SpectMorph
{

class LV2Plugin : public LV2Common
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

  LV2Plugin (double mix_freq);

  // SpectMorph stuff
  double          mix_freq;
  MorphPlanPtr    plan;
  QMutex          new_plan_mutex;
  MorphPlanPtr    new_plan;
  MidiSynth       midi_synth;
  string          plan_str;

  void update_plan (const string& new_plan_str);
};

}

LV2Plugin::LV2Plugin (double mix_freq) :
  midi_in (NULL),
  gain (NULL),
  input (NULL),
  output (NULL),
  notify_port (NULL),
  log (NULL),
  schedule (NULL),
  mix_freq (mix_freq),
  plan (new MorphPlan()),
  midi_synth (mix_freq, 64)
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
  midi_synth.update_plan (plan);
}

void
LV2Plugin::update_plan (const string& new_plan_str)
{
  plan_str = new_plan_str;

  MorphPlanPtr new_plan = new MorphPlan();
  new_plan->set_plan_str (plan_str);

  // this might take a while, and cannot be used in audio thread
  MorphPlanSynth mp_synth (mix_freq);
  MorphPlanVoice *mp_voice = mp_synth.add_voice();
  mp_synth.update_plan (new_plan);

  MorphOutputModule *om = mp_voice->output();
  if (om)
    {
      om->retrigger (0, 440, 1);
      float s;
      float *values[1] = { &s };
      om->process (1, values, 1);
    }

  // install new plan
  QMutexLocker locker (&new_plan_mutex);
  this->new_plan = new_plan;
}

static LV2_Handle
instantiate (const LV2_Descriptor*     descriptor,
             double                    rate,
             const char*               bundle_path,
             const LV2_Feature* const* features)
{
  if (!sm_init_done())
    sm_init_plugin();

  LV2Plugin *self = new LV2Plugin (rate);

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
  LV2Plugin* self = (LV2Plugin*)instance;

  switch ((PortIndex)port)
    {
      case SPECTMORPH_MIDI_IN:    self->midi_in = (const LV2_Atom_Sequence*)data;
                                  break;
      case SPECTMORPH_GAIN:       self->gain = (const float*)data;
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

class WorkMsg
{
  char *m_plan_str;
public:
  WorkMsg (const char *plan_str)
  {
    m_plan_str = g_strdup (plan_str);
  }
  WorkMsg (uint32_t size, const void *data)
  {
    assert (size == sizeof (WorkMsg));
    memcpy (this, data, size);
  }
  void
  free()
  {
    g_free (m_plan_str);

    m_plan_str = NULL;
  }
  const char *
  plan_str()
  {
    return m_plan_str;
  }
};

static LV2_Worker_Status
work (LV2_Handle                  instance,
      LV2_Worker_Respond_Function respond,
      LV2_Worker_Respond_Handle   handle,
      uint32_t                    size,
      const void*                 data)
{
  LV2Plugin* self = (LV2Plugin*)instance;

  // Get new plan from message
  MorphPlanPtr  new_plan = new MorphPlan();
  WorkMsg       msg (size, data);

  self->update_plan (msg.plan_str());

  msg.free();
  return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status
work_response (LV2_Handle  instance,
               uint32_t    size,
               const void* data)
{
  return LV2_WORKER_SUCCESS;
}

static void
run (LV2_Handle instance, uint32_t n_samples)
{
  LV2Plugin* self = (LV2Plugin*)instance;

  if (self->new_plan_mutex.tryLock())
    {
      if (self->new_plan)
        {
          self->midi_synth.update_plan (self->new_plan);
          self->new_plan = NULL;
        }
      self->new_plan_mutex.unlock();
    }

  const float        gain   = *(self->gain);
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
          const uint8_t* msg = (const uint8_t*)(ev + 1);

          self->midi_synth.add_midi_event (ev->time.frames, msg);
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
                  // send plan change to worker
                  WorkMsg msg ((const char *) LV2_ATOM_BODY_CONST (value));

                  self->schedule->schedule_work (self->schedule->handle, sizeof (msg), &msg);
                }
            }
          if (obj->body.otype == self->uris.patch_Get)
            {
              lv2_atom_forge_frame_time(&self->forge, offset);

              self->write_set_plan (&self->forge, self->plan_str);
            }
        }
    }
  self->midi_synth.process (output, n_samples);

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
  delete static_cast <LV2Plugin *> (instance);
}

static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
  LV2Plugin* self = static_cast <LV2Plugin *> (instance);

  store (handle,
         self->uris.spectmorph_plan,
         self->plan_str.c_str(),
         self->plan_str.size() + 1,
         self->uris.atom_String,
         LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

  debug ("state save called: %s\n", self->plan_str.c_str());
  return LV2_STATE_SUCCESS;
}

static LV2_State_Status
restore(LV2_Handle                  instance,
        LV2_State_Retrieve_Function retrieve,
        LV2_State_Handle            handle,
        uint32_t                    flags,
        const LV2_Feature* const*   features)
{
  LV2Plugin* self = static_cast <LV2Plugin *> (instance);

  debug ("state restore called\n");

  size_t   size;
  uint32_t type;
  uint32_t valflags;

  const void* value = retrieve (handle, self->uris.spectmorph_plan, &size, &type, &valflags);
  if (value)
    {
      const char* plan_str = (const char*)value;

      debug ("state: %s\n", plan_str);
      self->update_plan (plan_str);
    }

  return LV2_STATE_SUCCESS;
}

static const void*
extension_data (const char* uri)
{
  static const LV2_State_Interface  state  = { save, restore };
  static const LV2_Worker_Interface worker = { work, work_response, NULL };

  if (!strcmp(uri, LV2_STATE__interface))
    {
      return &state;
    }
  else if (!strcmp(uri, LV2_WORKER__interface))
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
