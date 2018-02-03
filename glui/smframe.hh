// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_FRAME_HH
#define SPECTMORPH_FRAME_HH

namespace SpectMorph
{

struct Frame : public Widget
{
  Frame (Widget *parent, double x, double y, double width, double height)
    : Widget (parent, x, y, width, height)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    static const double radius  = 5;
    static const double degrees = 3.14159265 / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr,
               width - radius,
               radius,
               radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr,
                width - radius, height - radius,
               radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr,
               radius, height - radius,
               radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr,
                radius, radius,
               radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    // Draw border
    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);

}
};

}

#endif

