// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SIMPLE_LINES_HH
#define SPECTMORPH_SIMPLE_LINES_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class VLine : public Widget
{
  Color color;
  double line_width;
public:
  VLine (Widget *parent, Color color, double line_width) :
    Widget (parent),
    color (color),
    line_width (line_width)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    du.set_color (color);
    cairo_set_line_width (devent.cr, line_width);
    cairo_move_to (devent.cr, width() / 2, 0);
    cairo_line_to (devent.cr, width() / 2, height());
    cairo_stroke (devent.cr);
  }
};


class HLine : public Widget
{
  Color color;
  double line_width;
public:
  HLine (Widget *parent, Color color, double line_width) :
    Widget (parent),
    color (color),
    line_width (line_width)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    du.set_color (color);
    cairo_set_line_width (devent.cr, line_width);
    cairo_move_to (devent.cr, 0, height() / 2);
    cairo_line_to (devent.cr, width(), height() / 2);
    cairo_stroke (devent.cr);
  }
};

}

#endif
