// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
};

}
