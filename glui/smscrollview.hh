// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_SCROLLVIEW_HH
#define SPECTMORPH_SCROLLVIEW_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct ScrollView : public Widget
{
  double scroll_x = 0;
  double scroll_y = 0;
  ScrollView (Widget *parent) :
    Widget (parent)
  {
  }
  ScrollView *
  scroll_view() override
  {
    return this;
  }
  ScrollView *
  is_scroll_view() override
  {
    return this;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
    du.round_box (0, 0, width, height, 1, 1);
  }
};

}

#endif
