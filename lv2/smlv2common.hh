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
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/instance-access/instance-access.h"

#include <string>

#define SPECTMORPH_URI      "http://spectmorph.org/plugins/spectmorph"
#define SPECTMORPH_UI_URI   SPECTMORPH_URI "#ui"

#define SPECTMORPH__plan    SPECTMORPH_URI "#plan"
#define SPECTMORPH__volume  SPECTMORPH_URI "#volume"

#ifndef LV2_STATE__StateChanged
#define LV2_STATE__StateChanged LV2_STATE_PREFIX "StateChanged"
#endif

namespace SpectMorph
{

class LV2Common
{
public:
  struct {
    LV2_URID atom_eventTransfer;
    LV2_URID atom_URID;
    LV2_URID atom_Blank;
    LV2_URID atom_Bool;
    LV2_URID atom_Double;
    LV2_URID atom_Float;
    LV2_URID atom_Int;
    LV2_URID atom_Long;
    LV2_URID atom_Object;
    LV2_URID atom_String;
    LV2_URID midi_MidiEvent;
    LV2_URID spectmorph_plan;
    LV2_URID spectmorph_volume;
    LV2_URID state_StateChanged;
    LV2_URID time_bar;
    LV2_URID time_barBeat;
    LV2_URID time_beatUnit;
    LV2_URID time_beatsPerBar;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_Position;
  } uris;
  LV2_URID_Map* map;

  void
  init_map (LV2_URID_Map *map)
  {
    this->map = map;

    uris.atom_eventTransfer = map->map (map->handle, LV2_ATOM__eventTransfer);
    uris.atom_URID          = map->map (map->handle, LV2_ATOM__URID);
    uris.atom_Blank         = map->map (map->handle, LV2_ATOM__Blank);
    uris.atom_Bool          = map->map (map->handle, LV2_ATOM__Bool);
    uris.atom_Double        = map->map (map->handle, LV2_ATOM__Double);
    uris.atom_Float         = map->map (map->handle, LV2_ATOM__Float);
    uris.atom_Int           = map->map (map->handle, LV2_ATOM__Int);
    uris.atom_Long          = map->map (map->handle, LV2_ATOM__Long);
    uris.atom_Object        = map->map (map->handle, LV2_ATOM__Object);
    uris.atom_String        = map->map (map->handle, LV2_ATOM__String);
    uris.midi_MidiEvent     = map->map (map->handle, LV2_MIDI__MidiEvent);
    uris.spectmorph_plan    = map->map (map->handle, SPECTMORPH__plan);
    uris.spectmorph_volume  = map->map (map->handle, SPECTMORPH__volume);
    uris.state_StateChanged = map->map (map->handle, LV2_STATE__StateChanged);
    uris.time_bar           = map->map (map->handle, LV2_TIME__bar);
    uris.time_barBeat       = map->map (map->handle, LV2_TIME__barBeat);
    uris.time_beatUnit      = map->map (map->handle, LV2_TIME__beatUnit);
    uris.time_beatsPerBar   = map->map (map->handle, LV2_TIME__beatsPerBar);
    uris.time_beatsPerMinute  = map->map (map->handle, LV2_TIME__beatsPerMinute);
    uris.time_Position        = map->map (map->handle, LV2_TIME__Position);
  }
};

}

#endif /* SPECTMORPH_LV2_COMMON_HH */
