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
#include "lv2/lv2plug.in/ns/ext/instance-access/instance-access.h"

#include <string>

#define SPECTMORPH_URI      "http://spectmorph.org/plugins/spectmorph"
#define SPECTMORPH_UI_URI   SPECTMORPH_URI "#ui"

#define SPECTMORPH__Get     SPECTMORPH_URI "#Get"
#define SPECTMORPH__Set     SPECTMORPH_URI "#Set"
#define SPECTMORPH__led     SPECTMORPH_URI "#led"
#define SPECTMORPH__plan    SPECTMORPH_URI "#plan"
#define SPECTMORPH__volume  SPECTMORPH_URI "#volume"
#define SPECTMORPH__Event   SPECTMORPH_URI "#Event"
#define SPECTMORPH__event   SPECTMORPH_URI "#event"

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
    LV2_URID spectmorph_Event;
    LV2_URID spectmorph_event;
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
    uris.spectmorph_Event   = map->map (map->handle, SPECTMORPH__Event);
    uris.spectmorph_event   = map->map (map->handle, SPECTMORPH__event);
  }
};

}

#endif /* SPECTMORPH_LV2_COMMON_HH */
