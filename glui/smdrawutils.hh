// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwidget.hh"
#include "smtextrenderer.hh"

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
  text (const std::string& text, double x, double y, double width, double height,
        TextAlign align = TextAlign::LEFT, Orientation orientation = Orientation::HORIZONTAL)
  {
    // draw label

#if DEBUG_EXTENTS
    cairo_set_font_size (cr, 11.0);
    select_font_face (bold);

    cairo_font_extents_t font_extents_cairo;
    cairo_font_extents (cr, &font_extents_cairo);

    cairo_text_extents_t extents_cairo;
    cairo_text_extents (cr, text.c_str(), &extents_cairo);
#endif

    cairo_save (cr);

    cairo_matrix_t mat;
    cairo_get_matrix (cr, &mat);

    double s = (mat.xx + mat.yy) / 2;

    TextExtents extents;
    FontExtents font_extents;

    cairo_surface_t *text_surface = TextRenderer::the()->text_to_surface (s, bold, text, &font_extents, &extents, TextRenderer::Mode::RENDER_SURFACE);

#if (DEBUG_EXTENTS)
    printf ("\n");
    printf ("x_bearing %f %f\n", extents.x_bearing, extents_cairo.x_bearing);
    printf ("y_bearing %f %f\n", extents.y_bearing, extents_cairo.y_bearing);
    printf ("width %f %f\n", extents.width, extents_cairo.width);
    printf ("height %f %f\n", extents.height, extents_cairo.height);
    printf ("x_advance %f %f\n", extents.x_advance, extents_cairo.x_advance);
    // printf ("y_advance %f %f\n", extents.y_advance, extents_cairo.y_advance);
    printf ("\n");
    printf ("ascent %f %f\n", font_extents.ascent, font_extents_cairo.ascent);
    printf ("descent %f %f\n", font_extents.descent, font_extents_cairo.descent);
    printf ("height %f %f\n", font_extents.height, font_extents_cairo.height);
    printf ("max_x_advance %f %f\n", font_extents.max_x_advance, font_extents_cairo.max_x_advance);
    // printf ("max_y_advance %f %f\n", font_extents.max_y_advance, font_extents_cairo.max_y_advance);
#endif

    if (orientation == Orientation::HORIZONTAL)
      {
        double fy = y + height / 2 - font_extents.descent + font_extents.height / 2;
        switch (align)
          {
            case TextAlign::LEFT:   cairo_translate (cr, x, fy);
                                    break;
            case TextAlign::CENTER: cairo_translate (cr, x + (width / 2) - extents.x_bearing - extents.width / 2, fy);
                                    break;
            case TextAlign::RIGHT:  cairo_translate (cr, x + width - extents.x_bearing - extents.width, fy);
                                    break;
          }
      }
    else
      {
        double fx = x + width / 2 + font_extents.height / 2 - font_extents.descent;
        switch (align)
          {
            case TextAlign::LEFT:   cairo_translate (cr, fx, y + height);
                                    break;
            case TextAlign::CENTER: cairo_translate (cr, fx, y + height / 2 + extents.x_bearing + extents.width / 2);
                                    break;
            case TextAlign::RIGHT:  cairo_translate (cr, fx, y + extents.x_bearing + extents.width);
                                    break;
          }
      }
    // use simple matrix (no scaling) for drawing text_surface
    cairo_get_matrix (cr, &mat);
    mat.xx = 1.0;
    mat.yy = 1.0;
    mat.xy = 0.0;
    mat.yx = 0.0;
    cairo_set_matrix (cr, &mat);

    if (orientation == Orientation::VERTICAL)
      cairo_rotate (cr, -M_PI / 2);

    double ux = extents.x_bearing * s, uy = extents.y_bearing * s;

    // snap to integer device pixels
    cairo_user_to_device (cr, &ux, &uy);
    ux = round (ux);
    uy = round (uy);
    cairo_device_to_user (cr, &ux, &uy);

    cairo_mask_surface (cr, text_surface, ux, uy);
    cairo_surface_destroy (text_surface);

    cairo_restore (cr);
  }
  TextExtents
  text_extents (const std::string& text)
  {
    cairo_matrix_t mat;
    cairo_get_matrix (cr, &mat);
    double s = (mat.xx + mat.yy) / 2;
    TextExtents extents;

    TextRenderer::the()->text_to_surface (s, bold, text, nullptr, &extents, TextRenderer::Mode::EXTENTS_ONLY);

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
  static TextExtents static_text_extents (Window *window, const std::string& text); /* static version: without instance */
};

}
