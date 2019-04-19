// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdlib.h>
#include <stdio.h>

#include "smlv2common.hh"
#include "smlv2ui.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smutils.hh"
#include "smmain.hh"

#include <mutex>

using namespace SpectMorph;

using std::vector;
using std::string;

#define DEBUG 1

static FILE       *debug_file = NULL;
static std::mutex  debug_mutex;

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
      std::lock_guard<std::mutex> locker (debug_mutex);

      if (!debug_file)
        debug_file = fopen ("/tmp/smlv2ui.log", "w");

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", string_vprintf (fmt, ap).c_str());
      va_end (ap);
      fflush (debug_file);
    }
}

LV2UI::LV2UI (PuglNativeWindow parent_win_id, LV2UI_Resize *ui_resize) :
  ui_resize (ui_resize)
  //morph_plan (new MorphPlan ())
{
  Project *fake = new Project();
  morph_plan = new MorphPlan (*fake);
  window = new MorphPlanWindow (event_loop, "SpectMorph LV2", parent_win_id, /* resize */ false, morph_plan, this);

  connect (window->control_widget()->signal_volume_changed, this, &LV2UI::on_volume_changed);
  connect (morph_plan->signal_plan_changed, this, &LV2UI::on_plan_changed);
  connect (window->signal_update_size, this, &LV2UI::on_update_window_size);

  window->show();
}

void
LV2UI::on_update_window_size()
{
  if (ui_resize)
    {
      int width, height;
      window->get_scaled_size (&width, &height);

      ui_resize->ui_resize (ui_resize->handle, width, height);
    }
}

LV2UI::~LV2UI()
{
  delete window;
  window = nullptr;
}

void
LV2UI::idle()
{
  event_loop.process_events();
}

void
LV2UI::on_plan_changed()
{
  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo);

  string plan_str = HexString::encode (data);

  // if we know that the plugin already has this plan, don't send it again
  if (plan_str == current_plan)
    return;
  current_plan = plan_str;

  const size_t OBJ_BUF_SIZE = plan_str.size() + 1024;
  uint8_t obj_buf[OBJ_BUF_SIZE];
  lv2_atom_forge_set_buffer (&forge, obj_buf, OBJ_BUF_SIZE);

  const LV2_Atom* msg = write_set_plan (&forge, plan_str);

  write (controller, 0, lv2_atom_total_size (msg),
         uris.atom_eventTransfer,
         msg);
}

void
LV2UI::synth_take_control_event (SynthControlEvent *event)
{
#if 0 //XXX
  string inst_edit_str = event->to_string();
  delete event;

  const size_t OBJ_BUF_SIZE = inst_edit_str.size() + 1024;
  uint8_t obj_buf[OBJ_BUF_SIZE];
  lv2_atom_forge_set_buffer (&forge, obj_buf, OBJ_BUF_SIZE);

  const LV2_Atom *msg = write_event (&forge, inst_edit_str);

  write (controller, 0, lv2_atom_total_size (msg),
         uris.atom_eventTransfer,
         msg);
#endif
}

void
LV2UI::on_volume_changed (double new_volume)
{
  vector<uint8_t> obj_buf (512);

  lv2_atom_forge_set_buffer (&forge, &obj_buf[0], obj_buf.size());

  const LV2_Atom* msg = write_set_volume (&forge, new_volume);

  write (controller, 0, lv2_atom_total_size (msg), uris.atom_eventTransfer, msg);
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor*   descriptor,
            const char*               plugin_uri,
            const char*               bundle_path,
            LV2UI_Write_Function      write_function,
            LV2UI_Controller          controller,
            LV2UI_Widget*             widget,
            const LV2_Feature* const* features)
{
  debug ("instantiate called for ui\n");

  if (!sm_init_done())
    sm_init_plugin();


  PuglNativeWindow parent_win_id = 0;
  LV2_URID_Map* map    = nullptr;
  LV2UI_Resize *ui_resize = nullptr;
  for (int i = 0; features[i]; i++)
    {
      if (!strcmp (features[i]->URI, LV2_URID__map))
        {
          map = (LV2_URID_Map*)features[i]->data;
        }
      else if (!strcmp (features[i]->URI, LV2_UI__parent))
        {
          parent_win_id = (PuglNativeWindow)features[i]->data;
          debug ("Parent X11 ID %i\n", parent_win_id);
        }
      else if (!strcmp (features[i]->URI, LV2_UI__resize))
        {
          ui_resize = (LV2UI_Resize*)features[i]->data;
        }
      else if (!strcmp (features[i]->URI, LV2_INSTANCE_ACCESS_URI))
        {
          debug ("instance access: %p\n", features[i]->data);
        }
    }
  if (!map)
    {
      return nullptr; // host bug, we need this feature
    }
  LV2UI *ui = new LV2UI (parent_win_id, ui_resize);
  ui->init_map (map);

  ui->write = write_function;
  ui->controller = controller;

  lv2_atom_forge_init (&ui->forge, ui->map);

  // Request state (volume, plan) from plugin
  uint8_t get_buf[512];
  lv2_atom_forge_set_buffer(&ui->forge, get_buf, sizeof(get_buf));

  LV2_Atom_Forge_Frame frame;
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->forge, &frame, 0, ui->uris.spectmorph_Get);
  lv2_atom_forge_pop (&ui->forge, &frame);

  ui->write(ui->controller, 0, lv2_atom_total_size(msg),
            ui->uris.atom_eventTransfer,
            msg);

  *widget = (void *)ui->window->native_window();

  /* set initial window size */
  ui->on_update_window_size();

  return ui;
}

static void
cleanup (LV2UI_Handle handle)
{
  debug ("cleanup called for ui\n");

  LV2UI *ui = static_cast <LV2UI *> (handle);
  delete ui;
}

void
LV2UI::port_event (uint32_t     port_index,
                   uint32_t     buffer_size,
                   uint32_t     format,
                   const void*  buffer)
{
  if (format == uris.atom_eventTransfer)
    {
      const LV2_Atom* atom = (const LV2_Atom*)buffer;
      if (lv2_atom_forge_is_object_type (&forge, atom->type))
        {
          const LV2_Atom_Object* obj      = (const LV2_Atom_Object*)atom;
          if (obj->body.otype == uris.spectmorph_Set)
            {
              const char  *plan_str;
              const float *volume_ptr;
              const int   *led_ptr;
              if (read_set (obj, &plan_str, &volume_ptr, &led_ptr))
                {
                  if (plan_str)
                    {
                      current_plan = plan_str; // if we received a plan, don't send the same thing back
                      morph_plan->set_plan_str (current_plan);
                    }
                  if (volume_ptr)
                    window->control_widget()->set_volume (*volume_ptr);
                  if (led_ptr)
                    window->control_widget()->set_led (*led_ptr);
                }
            }
          else if (obj->body.otype == uris.spectmorph_Event)
            {
              const char *event_str = nullptr;
              if (read_event (obj, &event_str))
                {
                  if (event_str)
                    notify_events.push_back (event_str);
                }
            }
          else
            {
              fprintf (stderr, "Ignoring unknown message type %d\n", obj->body.otype);
              return;
            }
        }
    }
}

vector<string>
LV2UI::notify_take_events()
{
  return std::move (notify_events);
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  LV2UI *ui = static_cast <LV2UI *> (handle);
  ui->port_event (port_index, buffer_size, format, buffer);

}

static int
idle (LV2UI_Handle handle)
{
  LV2UI* ui = (LV2UI*) handle;

  ui->idle();
  return 0;
}

static const LV2UI_Idle_Interface idle_iface = { idle };

static const void*
extension_data (const char* uri)
{
  // could implement show interface

  if (!strcmp(uri, LV2_UI__idleInterface)) {
    return &idle_iface;
  }
  return nullptr;
}

static const LV2UI_Descriptor descriptor = {
  SPECTMORPH_UI_URI,
  instantiate,
  cleanup,
  port_event,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor*
lv2ui_descriptor (uint32_t index)
{
  switch (index)
    {
      case 0:   return &descriptor;
      default:  return NULL;
    }
}
