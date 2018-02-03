// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_DRAWUTILS_HH
#define SPECTMORPH_DRAWUTILS_HH

namespace SpectMorph
{

struct DrawUtils
{
  cairo_t *cr;

  DrawUtils (cairo_t *cr) :
    cr (cr)
  {
  }
  void
  round_box (double x, double y, double width, double height, double line_width, double radius)
  {
    static const double degrees = 3.14159265 / 180.0;
    static const double lw_2 = line_width / 2;

    cairo_new_sub_path (cr);
    cairo_arc (cr,
               x + width - lw_2 - radius,
               y + lw_2 + radius,
               radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr,
               x + width - lw_2 - radius,
               y + height - lw_2 - radius,
               radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr,
               x + lw_2 + radius,
               y + height - lw_2 - radius,
               radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr,
               x + lw_2 + radius,
               y + lw_2 + radius,
               radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    // Draw border
    cairo_set_line_width (cr, line_width);
    cairo_stroke (cr);
  }
  void
  text (const std::string& text, double x, double y, double width, double height)
  {
    // draw label
    cairo_set_font_size (cr, 11.0);

    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    double fy = height / 2 - font_extents.descent + font_extents.height / 2;
    //if (align == Align::LEFT)
      cairo_move_to (cr, x, fy + y);
    //else if (align == Align::CENTER)
      //cairo_move_to (cr, (width / 2) - extents.x_bearing - extents.width / 2, fy);
    //else
      //cairo_move_to (cr, width - extents.x_bearing - extents.width, fy);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_show_text (cr, text.c_str());
  }
};

}

#endif

