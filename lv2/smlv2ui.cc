// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdlib.h>
#include <stdio.h>

#include "smlv2common.hh"
#include "smlv2ui.hh"
#include "smmemout.hh"
#include "smhexstring.hh"

using namespace SpectMorph;

using std::vector;
using std::string;

LV2UI::LV2UI() :
  morph_plan (new MorphPlan())
{
  window = new MorphPlanWindow (morph_plan, "!title!");

  connect (morph_plan.c_ptr(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
}

void
LV2UI::on_plan_changed()
{
  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo);

  string plan_str = HexString::encode (data);

  const size_t OBJ_BUF_SIZE = plan_str.size() + 1024;
  uint8_t obj_buf[OBJ_BUF_SIZE];
  lv2_atom_forge_set_buffer (&forge, obj_buf, OBJ_BUF_SIZE);

  const LV2_Atom* msg = write_set_file (&forge, plan_str.c_str(), plan_str.size());

  write (controller, 0, lv2_atom_total_size (msg),
         uris.atom_eventTransfer,
         msg);
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
  if (!sm_init_done())
    sm_init_plugin();

  LV2UI *ui = new LV2UI();

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
      delete ui;
      return NULL; // host bug, we need this feature
    }
  ui->init_map (map);

  ui->write = write_function;
  ui->controller = controller;

  lv2_atom_forge_init (&ui->forge, ui->map);

  // Request state (filename) from plugin
  uint8_t get_buf[512];
  lv2_atom_forge_set_buffer(&ui->forge, get_buf, sizeof(get_buf));

  LV2_Atom_Forge_Frame frame;
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->forge, &frame, 0, ui->uris.patch_Get);
  lv2_atom_forge_pop (&ui->forge, &frame);

  ui->write(ui->controller, 0, lv2_atom_total_size(msg),
            ui->uris.atom_eventTransfer,
            msg);

  *widget = ui->window;
  return ui;
}

static void
cleanup (LV2UI_Handle handle)
{
  LV2UI *ui = static_cast <LV2UI *> (handle);
  delete ui;
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  LV2UI *ui = static_cast <LV2UI *> (handle);

  if (format == ui->uris.atom_eventTransfer)
    {
      const LV2_Atom* atom = (const LV2_Atom*)buffer;
      if (lv2_atom_forge_is_object_type (&ui->forge, atom->type))
        {
          const LV2_Atom_Object* obj      = (const LV2_Atom_Object*)atom;
          const LV2_Atom*        file_uri = ui->read_set_file (obj);
          if (!file_uri)
            {
              fprintf(stderr, "Unknown message sent to UI.\n");
              return;
            }

          const char* uri = (const char*)LV2_ATOM_BODY_CONST(file_uri);
          ui->morph_plan->set_plan_str (uri);
        }
    }
}

static const void*
extension_data (const char* uri)
{
  // could implement show|idle interface
  return NULL;
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
