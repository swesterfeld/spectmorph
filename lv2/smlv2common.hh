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

#include <string>

#define SPECTMORPH_URI      "http://spectmorph.org/plugins/spectmorph"
#define SPECTMORPH_UI_URI   SPECTMORPH_URI "#ui"

#define SPECTMORPH__Get     SPECTMORPH_URI "#Get"
#define SPECTMORPH__Set     SPECTMORPH_URI "#Set"
#define SPECTMORPH__led     SPECTMORPH_URI "#led"
#define SPECTMORPH__plan    SPECTMORPH_URI "#plan"
#define SPECTMORPH__volume  SPECTMORPH_URI "#volume"

namespace SpectMorph
{

class LV2Common
{
public:
  struct {
    LV2_URID atom_eventTransfer;
    LV2_URID atom_URID;
    LV2_URID atom_Bool;
    LV2_URID atom_Float;
    LV2_URID atom_String;
    LV2_URID midi_MidiEvent;
    LV2_URID spectmorph_Get;
    LV2_URID spectmorph_Set;
    LV2_URID spectmorph_led;
    LV2_URID spectmorph_plan;
    LV2_URID spectmorph_volume;
  } uris;
  LV2_URID_Map* map;

  void
  init_map (LV2_URID_Map *map)
  {
    this->map = map;

    uris.atom_eventTransfer = map->map (map->handle, LV2_ATOM__eventTransfer);
    uris.atom_URID          = map->map (map->handle, LV2_ATOM__URID);
    uris.atom_Bool          = map->map (map->handle, LV2_ATOM__Bool);
    uris.atom_Float         = map->map (map->handle, LV2_ATOM__Float);
    uris.atom_String        = map->map (map->handle, LV2_ATOM__String);
    uris.midi_MidiEvent     = map->map (map->handle, LV2_MIDI__MidiEvent);
    uris.spectmorph_Get     = map->map (map->handle, SPECTMORPH__Get);
    uris.spectmorph_Set     = map->map (map->handle, SPECTMORPH__Set);
    uris.spectmorph_led     = map->map (map->handle, SPECTMORPH__led);
    uris.spectmorph_plan    = map->map (map->handle, SPECTMORPH__plan);
    uris.spectmorph_volume  = map->map (map->handle, SPECTMORPH__volume);
  }

  bool
  read_set (const LV2_Atom_Object* obj, const char **plan_str, const float **volume, const int **led)
  {
    const LV2_Atom* led_property    = nullptr;
    const LV2_Atom* plan_property   = nullptr;
    const LV2_Atom* volume_property = nullptr;

    *led      = nullptr;
    *plan_str = nullptr;
    *volume   = nullptr;

    lv2_atom_object_get (obj, uris.spectmorph_plan,   &plan_property,
                              uris.spectmorph_volume, &volume_property,
                              uris.spectmorph_led,    &led_property, 0);

    if (!plan_property && !volume_property && !led_property)
      {
        fprintf(stderr, "Malformed set message has no body.\n");
        return false;
      }
    if (plan_property && plan_property->type == uris.atom_String)
      {
        *plan_str = (const char*) LV2_ATOM_BODY_CONST (plan_property);
      }
    if (volume_property && volume_property->type == uris.atom_Float)
      {
        *volume = &((LV2_Atom_Float*)volume_property)->body;
      }
    if (led_property && led_property->type == uris.atom_Bool)
      {
        *led = &((LV2_Atom_Bool*)led_property)->body; // LV2_Atom_Bool must be int
      }
    return true;
  }

  LV2_Atom*
  write_set_plan (LV2_Atom_Forge* forge, const std::string& plan)
  {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*) lv2_atom_forge_object (forge, &frame, 0, uris.spectmorph_Set);

    lv2_atom_forge_key    (forge, uris.spectmorph_plan);
    lv2_atom_forge_string (forge, plan.c_str(), plan.size());

    lv2_atom_forge_pop (forge, &frame);

    return set;
  }

  LV2_Atom*
  write_set_volume (LV2_Atom_Forge* forge, float volume)
  {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*) lv2_atom_forge_object (forge, &frame, 0, uris.spectmorph_Set);

    lv2_atom_forge_key    (forge, uris.spectmorph_volume);
    lv2_atom_forge_float  (forge, volume);

    lv2_atom_forge_pop (forge, &frame);

    return set;
  }
  LV2_Atom*
  write_set_led (LV2_Atom_Forge* forge, bool state)
  {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*) lv2_atom_forge_object (forge, &frame, 0, uris.spectmorph_Set);

    lv2_atom_forge_key    (forge, uris.spectmorph_led);
    lv2_atom_forge_bool   (forge, state);

    lv2_atom_forge_pop (forge, &frame);

    return set;
  }
  LV2_Atom*
  write_set_all (LV2_Atom_Forge* forge, const std::string& plan, float volume, bool led_state)
  {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*) lv2_atom_forge_object (forge, &frame, 0, uris.spectmorph_Set);

    lv2_atom_forge_key    (forge, uris.spectmorph_plan);
    lv2_atom_forge_string (forge, plan.c_str(), plan.size());
    lv2_atom_forge_key    (forge, uris.spectmorph_volume);
    lv2_atom_forge_float  (forge, volume);
    lv2_atom_forge_key    (forge, uris.spectmorph_led);
    lv2_atom_forge_bool   (forge, led_state);

    lv2_atom_forge_pop (forge, &frame);

    return set;
  }
};

}

#endif /* SPECTMORPH_LV2_COMMON_HH */
