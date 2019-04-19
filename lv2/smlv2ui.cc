// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdlib.h>
#include <stdio.h>

#include "smlv2common.hh"
#include "smlv2ui.hh"
#include "smlv2plugin.hh"
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

LV2UI::LV2UI (PuglNativeWindow parent_win_id, LV2UI_Resize *ui_resize, LV2Plugin *plugin) :
  plugin (plugin),
  ui_resize (ui_resize),
  morph_plan (new MorphPlan (plugin->project))
{
  morph_plan->set_plan_str (plugin->plan_str);

  window = new MorphPlanWindow (event_loop, "SpectMorph LV2", parent_win_id, /* resize */ false, morph_plan, plugin);
  window->control_widget()->set_volume (plugin->volume);

  connect (window->control_widget()->signal_volume_changed, this, &LV2UI::on_volume_changed);
  connect (morph_plan->signal_plan_changed, this, &LV2UI::on_plan_changed);
  connect (window->signal_update_size, this, &LV2UI::on_update_window_size);
  connect (plugin->signal_post_load, this, &LV2UI::on_post_load);

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
  plugin->update_plan (plan_str);
}

void
LV2UI::on_volume_changed (double new_volume)
{
  plugin->set_volume (new_volume);
}

void
LV2UI::on_post_load()
{
  morph_plan->set_plan_str (plugin->plan_str);
  window->control_widget()->set_volume (plugin->volume);
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

  LV2Plugin *plugin = nullptr;
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
          plugin = (LV2Plugin *) features[i]->data;
        }
    }
  if (!map)
    {
      return nullptr; // host bug, we need this feature
    }
  LV2UI *ui = new LV2UI (parent_win_id, ui_resize, plugin);
  ui->init_map (map);

  ui->write = write_function;
  ui->controller = controller;

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
