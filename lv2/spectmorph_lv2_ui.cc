// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdlib.h>
#include <stdio.h>

#include <QWidget>
#include <QLabel>

#include "smmorphplanwindow.hh"
#include "smlv2common.hh"

using namespace SpectMorph;

class SpectMorphLV2UI : public SpectMorph::LV2Common
{
public:
  SpectMorphLV2UI();

  MorphPlanWindow      *window;
  MorphPlanPtr          morph_plan;

  LV2_Atom_Forge        forge;
  LV2UI_Write_Function  write;
  LV2UI_Controller      controller;
};

SpectMorphLV2UI::SpectMorphLV2UI() :
  morph_plan (new MorphPlan())
{
  window = new MorphPlanWindow (morph_plan, "!title!");
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

  SpectMorphLV2UI *ui = new SpectMorphLV2UI();

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
  SpectMorphLV2UI *ui = static_cast <SpectMorphLV2UI *> (handle);
  delete ui;
}

/**
 * Get the file path from a message like:
 * []
 *     a patch:Set ;
 *     patch:property eg:sample ;
 *     patch:value </home/me/foo.wav> .
 */
static inline const LV2_Atom*
read_set_file(const SpectMorphLV2UI* self,
              const LV2_Atom_Object* obj)
{
  if (obj->body.otype != self->uris.patch_Set)
    {
      fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
      return NULL;
    }

  /* Get property URI. */
  const LV2_Atom* property = NULL;
  lv2_atom_object_get(obj, self->uris.patch_property, &property, 0);
  if (!property)
    {
      fprintf(stderr, "Malformed set message has no body.\n");
      return NULL;
    }
  else if (property->type != self->uris.atom_URID)
    {
      fprintf(stderr, "Malformed set message has non-URID property.\n");
      return NULL;
    }
  else if (((const LV2_Atom_URID*)property)->body != self->uris.spectmorph_plan)
    {
      fprintf(stderr, "Set message for unknown property.\n");
      return NULL;
    }

  /* Get value. */
  const LV2_Atom* file_path = NULL;
  lv2_atom_object_get(obj, self->uris.patch_value, &file_path, 0);
  if (!file_path)
    {
      fprintf(stderr, "Malformed set message has no value.\n");
      return NULL;
    }
  else if (file_path->type != self->uris.atom_Path)
    {
      fprintf(stderr, "Set message value is not a Path.\n");
      return NULL;
    }
  return file_path;
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  SpectMorphLV2UI *ui = static_cast <SpectMorphLV2UI *> (handle);

  fprintf (stderr, "port_event\n");
  if (format == ui->uris.atom_eventTransfer)
    {
      printf ("format is atom_eventTransfer\n");
      const LV2_Atom* atom = (const LV2_Atom*)buffer;
      if (lv2_atom_forge_is_object_type (&ui->forge, atom->type))
        {
          printf ("is object type\n");
          const LV2_Atom_Object* obj      = (const LV2_Atom_Object*)atom;
          const LV2_Atom*        file_uri = read_set_file (ui, obj);
          if (!file_uri)
            {
              fprintf(stderr, "Unknown message sent to UI.\n");
              return;
            }

          const char* uri = (const char*)LV2_ATOM_BODY_CONST(file_uri);
          printf ("ui: received uri: %s\n", uri);
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
