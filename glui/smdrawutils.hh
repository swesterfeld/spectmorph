// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_DRAWUTILS_HH
#define SPECTMORPH_DRAWUTILS_HH

#include "smwidget.hh"

namespace SpectMorph
{

struct DrawUtils
{
  cairo_t *cr;
  bool bold = false;

  DrawUtils (cairo_t *cr) :
    cr (cr)
  {
  }
  void
  round_box (double x, double y, double width, double height, double line_width, double radius,
             Color frame_color, Color fill_color = Color::null())
  {
    constexpr double degrees = 3.14159265 / 180.0;
    const double lw_2 = line_width / 2;

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

    if (fill_color)
      {
        // Draw background
        set_color (fill_color);

        if (frame_color) /* preserve path for frame */
          cairo_fill_preserve (cr);
        else
          cairo_fill (cr);
      }
    // Draw border
    if (frame_color)
      {
        set_color (frame_color);
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);
      }
  }
  void
  round_box (const Rect& r, double line_width, double radius,
             Color frame_color, Color fill_color = Color::null())
  {
    round_box (r.x(), r.y(), r.width(), r.height(), line_width, radius, frame_color, fill_color);
  }
  void
  rect_fill (double x, double y, double width, double height, Color fill_color)
  {
    set_color (fill_color);
    cairo_rectangle (cr, x, y, width, height);
    cairo_fill (cr);
  }
  void
  circle (double x, double y, double radius, Color fill_color)
  {
    set_color (fill_color);
    cairo_arc (cr, x, y, radius, 0, 2 * M_PI);
    cairo_fill (cr);
  }
  void
  text (const std::string& text, double x, double y, double width, double height, TextAlign align = TextAlign::LEFT)
  {
    // draw label
    cairo_set_font_size (cr, 11.0);
    select_font_face (bold);

    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    double fy = height / 2 - font_extents.descent + font_extents.height / 2;
    switch (align)
      {
        case TextAlign::LEFT:   cairo_move_to (cr, x, fy + y);
                                break;
        case TextAlign::CENTER: cairo_move_to (cr, x + (width / 2) - extents.x_bearing - extents.width / 2, fy + y);
                                break;
        case TextAlign::RIGHT:  cairo_move_to (cr, x + width - extents.x_bearing - extents.width, fy + y);
                                break;
      }
    cairo_show_text (cr, text.c_str());
  }
  cairo_text_extents_t
  text_extents (const std::string& text)
  {
    cairo_set_font_size (cr, 11.0);
    select_font_face (bold);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);
    return extents;
  }
  double
  text_width (const std::string& text)
  {
    return text_extents (text).width;
  }
  void
  set_color (Color color)
  {
    cairo_set_source_rgb (cr, color.red(), color.green(), color.blue());
  }
  void select_font_face (bool bold);
  static double static_text_width (Window *window, const std::string& text); /* static version: without instance */
  static cairo_text_extents_t static_text_extents (Window *window, const std::string& text); /* static version: without instance */
};

}

#endif

