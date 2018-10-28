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

#include <mutex>

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

using namespace SpectMorph;
using std::string;
using std::vector;
using std::max;

#define DEBUG 0

static FILE       *debug_file = NULL;
static std::mutex  debug_mutex;

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
      std::lock_guard<std::mutex> locker (debug_mutex);

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
  SPECTMORPH_MIDI_IN    = 0,
  SPECTMORPH_CONTROL_1  = 1,
  SPECTMORPH_CONTROL_2  = 2,
  SPECTMORPH_LEFT_OUT   = 3,
  SPECTMORPH_RIGHT_OUT  = 4,
  SPECTMORPH_NOTIFY     = 5
};

namespace SpectMorph
{

class LV2Plugin : public LV2Common
{
public:
  // Port buffers
  const LV2_Atom_Sequence* midi_in;
  const float* control_1;
  const float* control_2;
  float*       left_out;
  float*       right_out;
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
  double          volume;
  std::mutex      new_plan_mutex;
  MorphPlanPtr    new_plan;
  MidiSynth       midi_synth;
  string          plan_str;
  bool            voices_active;
  bool            send_settings_to_ui;
  bool            inst_edit_changed;
  InstEditUpdate  inst_edit_update;

  void update_plan (const string& new_plan_str);
  void handle_event (const string& event_str);
};

}

LV2Plugin::LV2Plugin (double mix_freq) :
  midi_in (NULL),
  control_1 (NULL),
  control_2 (NULL),
  left_out (NULL),
  right_out (NULL),
  notify_port (NULL),
  log (NULL),
  schedule (NULL),
  mix_freq (mix_freq),
  midi_synth (mix_freq, 64)
{
  MorphPlanPtr plan = new MorphPlan();
  plan->load_default();

  vector<unsigned char> data;
  MemOut mo (&data);
  plan->save (&mo);
  plan_str = HexString::encode (data);

  debug ("SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());
  midi_synth.update_plan (plan);

  volume = -6;            // default volume (dB)
  voices_active = false;  // no note being played right now
  send_settings_to_ui = false;
}

void
LV2Plugin::update_plan (const string& new_plan_str)
{
  MorphPlanPtr new_plan = new MorphPlan();
  new_plan->set_plan_str (new_plan_str);

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
  std::lock_guard<std::mutex> locker (new_plan_mutex);
  this->new_plan = new_plan;
  plan_str = new_plan_str;
}

void
LV2Plugin::handle_event (const string& event_str)
{
  string s;
  string in = event_str + "|";
  vector<string> vs;
  for (auto c : in)
    {
      if (c == '|')
        {
          vs.push_back (s);
          s = "";
        }
      else
        s += c;
    }
  if (vs[0] == "InstEditUpdate")
    {
      bool active = atoi (vs[1].c_str()) > 0;
      string filename = vs[2];
      bool original_samples = atoi (vs[3].c_str()) > 0;

      InstEditUpdate ie_update (active, filename, original_samples);
      ie_update.prepare();

      std::lock_guard<std::mutex> lg (new_plan_mutex);
      inst_edit_changed = true;
      inst_edit_update = ie_update;
    }
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
      case SPECTMORPH_CONTROL_1:  self->control_1 = (const float*)data;
                                  break;
      case SPECTMORPH_CONTROL_2:  self->control_2 = (const float*)data;
                                  break;
      case SPECTMORPH_LEFT_OUT:   self->left_out = (float*)data;
                                  break;
      case SPECTMORPH_RIGHT_OUT:  self->right_out = (float*)data;
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
public:
  enum class Type {
    PLAN,
    EVENT
  };
private:
  char *m_str = nullptr;
  Type  m_type;

public:
  WorkMsg (Type type, const char *str)
  {
    m_type = type;
    m_str  = g_strdup (str);
  }
  WorkMsg (uint32_t size, const void *data)
  {
    assert (size == sizeof (WorkMsg));
    memcpy (this, data, size);
  }
  void
  free()
  {
    g_free (m_str);

    m_str = NULL;
  }
  const char *
  str()
  {
    return m_str;
  }
  Type
  type()
  {
    return m_type;
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
  WorkMsg       msg (size, data);

  switch (msg.type())
    {
      case WorkMsg::Type::PLAN:   self->update_plan (msg.str());
                                  break;
      case WorkMsg::Type::EVENT:  self->handle_event (msg.str());
                                  break;
    }

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

  if (self->new_plan_mutex.try_lock())
    {
      if (self->new_plan)
        {
          self->midi_synth.update_plan (self->new_plan);
          self->new_plan = NULL;
        }
      if (self->inst_edit_changed)
        {
          self->inst_edit_update.run_rt (&self->midi_synth);
          self->inst_edit_changed = false;
        }
      self->new_plan_mutex.unlock();
    }

  const float        control_1  = *(self->control_1);
  const float        control_2  = *(self->control_2);
  float* const       left_out   = self->left_out;
  float* const       right_out  = self->right_out;

  uint32_t  offset = 0;

  // Set up forge to write directly to notify output port.
  const uint32_t notify_capacity = self->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&self->forge,
                            (uint8_t*)self->notify_port,
                            notify_capacity);

  // Start a sequence in the notify output port.
  lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);

  // send new settings to ui after restore
  if (self->send_settings_to_ui)
    {
      std::lock_guard<std::mutex> locker (self->new_plan_mutex); // need lock to access plan_str

      lv2_atom_forge_frame_time (&self->forge, offset);

      self->write_set_all (&self->forge, self->plan_str, self->volume, self->voices_active);
      self->send_settings_to_ui = false;
    }

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

          if (obj->body.otype == self->uris.spectmorph_Set)
            {
              const char  *plan_str;
              const float *volume_ptr;
              const int   *led_ptr;

              if (self->read_set (obj, &plan_str, &volume_ptr, &led_ptr))
                {
                  if (plan_str)
                    {
                      // send plan change to worker
                      WorkMsg msg (WorkMsg::Type::PLAN, plan_str);

                      self->schedule->schedule_work (self->schedule->handle, sizeof (msg), &msg);
                    }
                  if (volume_ptr)
                    {
                      self->volume = *volume_ptr;
                    }
                }
            }
          else if (obj->body.otype == self->uris.spectmorph_Get)
            {
              lv2_atom_forge_frame_time (&self->forge, offset);

              std::lock_guard<std::mutex> locker (self->new_plan_mutex); // need lock to access plan_str
              self->write_set_all (&self->forge, self->plan_str, self->volume, self->voices_active);
            }
          else if (obj->body.otype == self->uris.spectmorph_Event)
            {
              const char *event_str;

              if (self->read_event (obj, &event_str))
                {
                  if (event_str)
                    {
                      WorkMsg msg (WorkMsg::Type::EVENT, event_str);

                      self->schedule->schedule_work (self->schedule->handle, sizeof (msg), &msg);
                    }
                }
            }
        }
    }
  self->midi_synth.set_control_input (0, control_1);
  self->midi_synth.set_control_input (1, control_2);
  self->midi_synth.process (left_out, n_samples);

  // apply post volume
  const float v = (self->volume > -90) ? db_to_factor (self->volume) : 0;
  for (uint32_t i = 0; i < n_samples; i++)
    left_out[i] *= v;

  // proper stereo support will be added later
  std::copy (left_out, left_out + n_samples, right_out);

  // update led (all midi events processed by now)
  bool new_voices_active = self->midi_synth.active_voice_count() > 0;
  if (self->voices_active != new_voices_active)
    {
      lv2_atom_forge_frame_time (&self->forge, offset);

      self->write_set_led (&self->forge, new_voices_active);
      self->voices_active = new_voices_active;
    }
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

  std::lock_guard<std::mutex> locker (self->new_plan_mutex); // we read plan_str

  store (handle,
         self->uris.spectmorph_plan,
         self->plan_str.c_str(),
         self->plan_str.size() + 1,
         self->uris.atom_String,
         LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

  float f_volume = self->volume; // self->volume is double
  store (handle, self->uris.spectmorph_volume,
         (void*)&f_volume, sizeof (float),
         self->uris.atom_Float,
         LV2_STATE_IS_POD);

  debug ("state save called: %s\nstate volume: %f\n", self->plan_str.c_str(), self->volume);
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

  size_t      size;
  uint32_t    type;
  uint32_t    valflags;
  const void* value;

  value = retrieve (handle, self->uris.spectmorph_plan, &size, &type, &valflags);
  if (value && type == self->uris.atom_String)
    {
      const char *plan_str = (const char *)value;

      debug (" -> plan_str: %s\n", plan_str);
      self->update_plan (plan_str);
    }
  value = retrieve (handle, self->uris.spectmorph_volume, &size, &type, &valflags);
  if (value && size == sizeof (float) && type == self->uris.atom_Float)
    {
      self->volume = *((const float *) value);
      debug (" -> volume: %f\n", self->volume);
    }
  self->send_settings_to_ui = true;

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
