// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMBOBOX_HH
#define SPECTMORPH_COMBOBOX_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct ComboBox : public Widget
{
  ComboBox (Widget *parent)
    : Widget (parent, 0, 0, 100, 100)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
    du.round_box (0, space, width, height - 2 * space, 1, 5);
    du.text ("Trumpet", 10, 0, width - 10, height);
  }
};

}

#endif

