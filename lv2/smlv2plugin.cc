// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphwavsource.hh"
#include "smmidisynth.hh"
#include "smmain.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smutils.hh"
#include "smlv2common.hh"
#include "smlv2plugin.hh"

#include <mutex>


using namespace SpectMorph;
using std::string;
using std::vector;
using std::max;

enum PortIndex {
  SPECTMORPH_MIDI_IN    = 0,
  SPECTMORPH_CONTROL_1  = 1,
  SPECTMORPH_CONTROL_2  = 2,
  SPECTMORPH_LEFT_OUT   = 3,
  SPECTMORPH_RIGHT_OUT  = 4,
  SPECTMORPH_NOTIFY     = 5
};

LV2Plugin::LV2Plugin (double mix_freq) :
  midi_in (NULL),
  control_1 (NULL),
  control_2 (NULL),
  left_out (NULL),
  right_out (NULL),
  notify_port (NULL),
  log (NULL)
{
  project.set_mix_freq (mix_freq);
  project.set_storage_model (Project::StorageModel::REFERENCE);
  project.set_state_changed_notify (true);
}

#ifdef SM_STATIC_LINUX
static void
set_static_linux_data_dir()
{
  string pkg_data_dir = g_get_home_dir();
  pkg_data_dir += "/.spectmorph";

  LV2_DEBUG ("pkg data dir: '%s'\n", pkg_data_dir.c_str());
  sm_set_pkg_data_dir (pkg_data_dir);
}
#endif

static LV2_Handle
instantiate (const LV2_Descriptor*     descriptor,
             double                    rate,
             const char*               bundle_path,
             const LV2_Feature* const* features)
{
  sm_plugin_init();
#ifdef SM_STATIC_LINUX
  set_static_linux_data_dir();
#endif

  LV2Plugin *self = new LV2Plugin (rate);

  LV2_URID_Map* map = NULL;
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
    }
  if (!map)
    {
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

void
LV2Plugin::write_state_changed()
{
  LV2_Atom_Forge_Frame frame;

  lv2_atom_forge_frame_time (&forge, 0);
  lv2_atom_forge_object (&forge, &frame, 0, uris.state_StateChanged);
  lv2_atom_forge_pop (&forge, &frame);
}

static void
run (LV2_Handle instance, uint32_t n_samples)
{
  LV2Plugin* self = (LV2Plugin*)instance;

  const bool state_changed = self->project.try_update_synth();

  const float        control_1  = *(self->control_1);
  const float        control_2  = *(self->control_2);
  float* const       left_out   = self->left_out;
  float* const       right_out  = self->right_out;

  MidiSynth         *midi_synth = self->project.midi_synth();

  LV2_ATOM_SEQUENCE_FOREACH (self->midi_in, ev)
    {
      if (ev->body.type == self->uris.midi_MidiEvent)
        {
          const uint8_t* msg = (const uint8_t*)(ev + 1);

          midi_synth->add_midi_event (ev->time.frames, msg);
        }
      if (ev->body.type == self->uris.atom_Blank || ev->body.type == self->uris.atom_Object)
        {
          const LV2_Atom_Object *obj = (LV2_Atom_Object *) &ev->body;
          if (obj->body.otype == self->uris.time_Position)
            {
              LV2_Atom *beats_per_minute = nullptr;
              LV2_Atom *beats_per_bar = nullptr;
              LV2_Atom *bar = nullptr;
              LV2_Atom *bar_beat = nullptr;
              lv2_atom_object_get (obj,
                                   self->uris.time_beatsPerMinute, &beats_per_minute,
                                   self->uris.time_beatsPerBar, &beats_per_bar,
                                   self->uris.time_bar, &bar,
                                   self->uris.time_barBeat, &bar_beat,
                                   nullptr);
              if (beats_per_minute && beats_per_minute->type == self->uris.atom_Float)
                {
                  const float bpm = ((LV2_Atom_Float *) beats_per_minute)->body;
                  midi_synth->set_tempo (bpm);
                  LV2_DEBUG ("got bpm=%f\n", bpm);
                }
              double b = -1;
              if (bar && bar->type == self->uris.atom_Long)
                {
                  b = ((LV2_Atom_Long *) bar)->body;
                  LV2_DEBUG ("got bar=%f\n", b);
                }
              float bb = -1;
              if (bar_beat && bar_beat->type == self->uris.atom_Float)
                {
                  bb = ((LV2_Atom_Float *) bar_beat)->body;
                  LV2_DEBUG ("got bar_beat=%f\n", bb);
                }
              float bpb = -1;
              if (beats_per_bar && beats_per_bar->type == self->uris.atom_Float)
                {
                  bpb = ((LV2_Atom_Float *) beats_per_bar)->body;
                  LV2_DEBUG ("got bpb=%f\n", bpb);
                }
              if (b >= 0 && bpb >= 0 && bb >= 0)
                midi_synth->set_ppq_pos (b * bpb + bb);
            }
        }
    }
  midi_synth->set_control_input (0, control_1);
  midi_synth->set_control_input (1, control_2);
  midi_synth->process (left_out, n_samples);

  // proper stereo support will be added later
  std::copy (left_out, left_out + n_samples, right_out);

  // send LV2_STATE__StateChanged if project state was modified
  if (state_changed)
    {
      // Set up forge to write directly to notify output port.
      const uint32_t notify_capacity = self->notify_port->atom.size;
      lv2_atom_forge_set_buffer (&self->forge,
                                 (uint8_t*)self->notify_port,
                                 notify_capacity);

      LV2_Atom_Forge_Frame frame;
      lv2_atom_forge_sequence_head (&self->forge, &frame, 0);

      self->write_state_changed();

      // Close off sequence
      lv2_atom_forge_pop (&self->forge, &frame);
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
  sm_plugin_cleanup();
}

static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
  LV2Plugin* self = static_cast <LV2Plugin *> (instance);

  LV2_State_Map_Path *map_path = nullptr;
  for (int i = 0; features[i]; i++)
    {
      if (!strcmp (features[i]->URI, LV2_STATE__mapPath))
        {
          map_path = (LV2_State_Map_Path *)features[i]->data;
        }
    }
  auto abstract_path = [map_path](string path)
    {
      if (map_path)
        {
          char *abstract_path = map_path->abstract_path (map_path->handle, path.c_str());
          path = abstract_path;
          free (abstract_path);
        }
      return path;
    };

  /* storing WavSource operators will temporarily modify the plan
   *  -> ignore state changed events during save
   */
  self->project.set_state_changed_notify (false);
  string plan_str = self->project.save_plan_lv2 (abstract_path);
  self->project.set_state_changed_notify (true);

  store (handle,
         self->uris.spectmorph_plan,
         plan_str.c_str(),
         plan_str.size() + 1,
         self->uris.atom_String,
         LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

  float f_volume = self->project.volume();
  store (handle, self->uris.spectmorph_volume,
         (void*)&f_volume, sizeof (float),
         self->uris.atom_Float,
         LV2_STATE_IS_POD);

  LV2_DEBUG ("state save called: %s\nstate volume: %f\n", plan_str.c_str(), f_volume);
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

  LV2_DEBUG ("state restore called\n");

  size_t      size;
  uint32_t    type;
  uint32_t    valflags;
  const void* value;

  LV2_State_Map_Path *map_path = nullptr;
  for (int i = 0; features[i]; i++)
    {
      if (!strcmp (features[i]->URI, LV2_STATE__mapPath))
        {
          map_path = (LV2_State_Map_Path *)features[i]->data;
        }
    }
  auto absolute_path = [map_path](string path)
    {
      if (map_path)
        {
          char *absolute_path = map_path->absolute_path (map_path->handle, path.c_str());
          path = absolute_path;
          free (absolute_path);
        }
      return path;
    };

  /* state changed notifications should not be sent if state was changed due to restore */
  self->project.set_state_changed_notify (false);

  value = retrieve (handle, self->uris.spectmorph_plan, &size, &type, &valflags);
  if (value && type == self->uris.atom_String)
    {
      const char *plan_str = (const char *)value;
      LV2_DEBUG (" -> plan_str: %s\n", plan_str);

      self->project.load_plan_lv2 (absolute_path, plan_str);
    }
  value = retrieve (handle, self->uris.spectmorph_volume, &size, &type, &valflags);
  if (value && size == sizeof (float) && type == self->uris.atom_Float)
    {
      float volume = *((const float *) value);
      self->project.set_volume (volume);
      LV2_DEBUG (" -> volume: %f\n", volume);
    }

  self->project.set_state_changed_notify (true);
  return LV2_STATE_SUCCESS;
}

static const void*
extension_data (const char* uri)
{
  static const LV2_State_Interface  state  = { save, restore };

  if (!strcmp(uri, LV2_STATE__interface))
    {
      return &state;
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
