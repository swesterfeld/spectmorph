// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_DRAWUTILS_HH
#define SPECTMORPH_DRAWUTILS_HH

#include "smwidget.hh"
#include <ft2build.h>
#include FT_FREETYPE_H

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
    cairo_set_font_size (cr, 11.0);
    select_font_face (bold);

    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    static bool init = false;
    static FT_Library ft_library;
    static FT_Face face;
    if (!init)
      {
      // Initialize FreeType
        if (FT_Init_FreeType(&ft_library)) {
            fprintf(stderr, "Could not init FreeType\n");
            return;
        }

        // Load TTF font face
        if (FT_New_Face(ft_library, "/usr/local/spectmorph/share/spectmorph/fonts/dejavu-lgc-sans.ttf", 0, &face)) {
            fprintf(stderr, "Could not open font file\n");
            return;
        }
        init = true;
      }

    cairo_matrix_t m;
    cairo_get_matrix(cr, &m);
    double s = (m.xx + m.yy) / 2;

    // Set font size (pixels)
    FT_Set_Char_Size (face, 0, lrint (11 * 64 * s), 0, 0);

    if (orientation == Orientation::HORIZONTAL)
      {
#if 0
        double fy = y + height / 2 - font_extents.descent + font_extents.height / 2;
#endif
        switch (align)
          {
#if 0
            case TextAlign::LEFT:   cairo_move_to (cr, x, fy);
                                    break;
            case TextAlign::CENTER: cairo_move_to (cr, x + (width / 2) - extents.x_bearing - extents.width / 2, fy);
                                    break;
            case TextAlign::RIGHT:  cairo_move_to (cr, x + width - extents.x_bearing - extents.width, fy);
                                    break;
#endif
            case TextAlign::LEFT:   /* nothing */
                                    break;
            case TextAlign::CENTER: x += (width / 2) - extents.x_bearing - extents.width / 2;
                                    break;
            case TextAlign::RIGHT:  x += width - extents.x_bearing - extents.width;
                                    break;
          }
        //cairo_show_text (cr, text.c_str());
      }
    double fy = y + height / 2 - font_extents.descent + font_extents.height / 2;
    double pen_x = x, pen_y = fy;
#if 0
    double pen_x = 0;
    double pen_y = 0;
#endif

    for (auto p : text) /* TODO: utf 8 */
      {
        FT_UInt glyph_index = FT_Get_Char_Index (face, p);

        // Load and render glyph
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
            continue; // skip on error
        }
        FT_GlyphSlot g = face->glyph;
        FT_Bitmap *bmp = &g->bitmap;
              // Create Cairo image surface from FreeType bitmap (grayscale -> alpha mask)
        cairo_surface_t *glyph_surface =
            cairo_image_surface_create(CAIRO_FORMAT_A8, bmp->width, bmp->rows);

        unsigned char *dst = cairo_image_surface_get_data(glyph_surface);
        for (int y = 0; y < bmp->rows; y++) {
            memcpy(dst + y * cairo_image_surface_get_stride(glyph_surface),
                   bmp->buffer + y * bmp->pitch,
                   bmp->width);
        }
        cairo_surface_mark_dirty (glyph_surface);

        // Paint glyph at correct position
        cairo_save (cr);

        cairo_matrix_t mat;
        cairo_get_matrix (cr, &mat);

        // zero out scale/skew
        mat.xx = 1.0;
        mat.yy = 1.0;
        mat.xy = 0.0;
        mat.yx = 0.0;
        cairo_set_matrix (cr, &mat);

        double ux = (pen_x * s + g->bitmap_left);
        double uy = (pen_y * s - g->bitmap_top);

        // Snap to integer device pixels
        cairo_user_to_device (cr, &ux, &uy);
        ux = round (ux);
        uy = round (uy);
        cairo_device_to_user (cr, &ux, &uy);

        cairo_mask_surface (cr, glyph_surface, ux, uy);
        cairo_restore (cr);

        // Advance pen position
        pen_x += (g->advance.x / 64.0) / s; // 1/64 pixels
        cairo_surface_destroy (glyph_surface);
    }

#if 0
    // draw label
    cairo_set_font_size (cr, 11.0);
    select_font_face (bold);

    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    if (orientation == Orientation::HORIZONTAL)
      {
        double fy = y + height / 2 - font_extents.descent + font_extents.height / 2;
        switch (align)
          {
            case TextAlign::LEFT:   cairo_move_to (cr, x, fy);
                                    break;
            case TextAlign::CENTER: cairo_move_to (cr, x + (width / 2) - extents.x_bearing - extents.width / 2, fy);
                                    break;
            case TextAlign::RIGHT:  cairo_move_to (cr, x + width - extents.x_bearing - extents.width, fy);
                                    break;
          }
        cairo_show_text (cr, text.c_str());
      }
    else
      {
        double fx = x + width / 2 + font_extents.height / 2 - font_extents.descent;
        switch (align)
          {
            case TextAlign::LEFT:   cairo_move_to (cr, fx, y + height);
                                    break;
            case TextAlign::CENTER: cairo_move_to (cr, fx, y + height / 2 + extents.x_bearing + extents.width / 2);
                                    break;
            case TextAlign::RIGHT:  cairo_move_to (cr, fx, y + extents.x_bearing + extents.width);
                                    break;
        }
        cairo_save (cr);
        cairo_rotate (cr, -M_PI / 2);
        cairo_show_text (cr, text.c_str());
        cairo_restore (cr);
      }
#endif
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

