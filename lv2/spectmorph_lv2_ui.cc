// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdlib.h>
#include <stdio.h>

#include <QWidget>
#include <QLabel>

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#define SPECTMORPH_UI_URI "http://spectmorph.org/plugins/spectmorph#ui"

class SpectMorphLV2UI
{
public:
  SpectMorphLV2UI();

  QWidget        *widget;
};

SpectMorphLV2UI::SpectMorphLV2UI()
{
  widget = new QLabel("foo");
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
  SpectMorphLV2UI *ui = new SpectMorphLV2UI();

  *widget = ui->widget;
  return ui;
}

static void
cleanup (LV2UI_Handle handle)
{
  SpectMorphLV2UI *ui = new SpectMorphLV2UI();
  delete ui;
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  fprintf (stderr, "port_event\n");
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
