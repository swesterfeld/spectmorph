// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LV2_COMMON_HH
#define SPECTMORPH_LV2_COMMON_HH

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#define SPECTMORPH_URI    "http://spectmorph.org/plugins/spectmorph"
#define SPECTMORPH__plan  SPECTMORPH_URI "#plan"
#define SPECTMORPH_UI_URI SPECTMORPH_URI "#ui"

namespace SpectMorph
{

class LV2Common
{
public:
  struct {
    LV2_URID atom_eventTransfer;
    LV2_URID atom_URID;
    LV2_URID atom_Path;
    LV2_URID midi_MidiEvent;
    LV2_URID patch_Get;
    LV2_URID patch_Set;
    LV2_URID patch_property;
    LV2_URID patch_value;
    LV2_URID spectmorph_plan;
  } uris;
  LV2_URID_Map* map;

  void
  init_map (LV2_URID_Map *map)
  {
    this->map = map;

    uris.atom_eventTransfer = map->map (map->handle, LV2_ATOM__eventTransfer);
    uris.atom_URID          = map->map (map->handle, LV2_ATOM__URID);
    uris.atom_Path          = map->map (map->handle, LV2_ATOM__Path);
    uris.midi_MidiEvent     = map->map (map->handle, LV2_MIDI__MidiEvent);
    uris.patch_Get          = map->map (map->handle, LV2_PATCH__Get);
    uris.patch_Set          = map->map (map->handle, LV2_PATCH__Set);
    uris.patch_property     = map->map (map->handle, LV2_PATCH__property);
    uris.patch_value        = map->map (map->handle, LV2_PATCH__value);
    uris.spectmorph_plan    = map->map (map->handle, SPECTMORPH__plan);
  }
  LV2_Atom*
  write_set_file (LV2_Atom_Forge*    forge,
                  const char*        filename,
                  const uint32_t     filename_len)
  {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*) lv2_atom_forge_object (forge, &frame, 0, uris.patch_Set);

    lv2_atom_forge_key (forge,  uris.patch_property);
    lv2_atom_forge_urid (forge, uris.spectmorph_plan);
    lv2_atom_forge_key (forge,  uris.patch_value);
    lv2_atom_forge_path (forge, filename, filename_len);

    lv2_atom_forge_pop (forge, &frame);

    return set;
  }

  const LV2_Atom*
  read_set_file (const LV2_Atom_Object* obj)
  {
    if (obj->body.otype != uris.patch_Set)
      {
        fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
        return NULL;
      }

    /* Get property URI. */
    const LV2_Atom* property = NULL;
    lv2_atom_object_get(obj, uris.patch_property, &property, 0);
    if (!property)
      {
        fprintf(stderr, "Malformed set message has no body.\n");
        return NULL;
      }
    else if (property->type != uris.atom_URID)
      {
        fprintf(stderr, "Malformed set message has non-URID property.\n");
        return NULL;
      }
    else if (((const LV2_Atom_URID*)property)->body != uris.spectmorph_plan)
      {
        fprintf(stderr, "Set message for unknown property.\n");
        return NULL;
      }

    /* Get value. */
    const LV2_Atom* file_path = NULL;
    lv2_atom_object_get(obj, uris.patch_value, &file_path, 0);
    if (!file_path)
      {
        fprintf(stderr, "Malformed set message has no value.\n");
        return NULL;
      }
    else if (file_path->type != uris.atom_Path)
      {
        fprintf(stderr, "Set message value is not a Path.\n");
        return NULL;
      }
    return file_path;
  }
};

}

#endif /* SPECTMORPH_LV2_COMMON_HH */
