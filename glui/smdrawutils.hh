// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_DRAWUTILS_HH
#define SPECTMORPH_DRAWUTILS_HH

#include "smwidget.hh"
#include "smconfig.hh"
#include <map>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace SpectMorph
{

#define DEBUG_EXTENTS 0

struct FontExtents
{
  double ascent = 0;
  double descent = 0;
  double height = 0;
  double max_x_advance = 0;
//  double max_y_advance = 0; <- always zero
};

struct TextExtents
{
  double x_bearing = 0;
  double y_bearing = 0;
  double width = 0;
  double height = 0;
  double x_advance = 0;
//  double y_advance = 0; <- always zero
};

struct Glyph
{
  int bitmap_left;
  int bitmap_top;
  int bitmap_width;
  int bitmap_height;
  std::vector<uint8_t> bitmap;
  double advance_x;
};

class TextRenderer {
  std::unique_ptr<Config>               cfg;
  bool                                  ft_init_ok = false;
  FT_Library                            ft_library;

  bool
  load_font (FT_Face *out_face, bool bold) const
  {
    std::string filename = sm_get_install_dir (INSTALL_DIR_FONTS);
    if (bold)
      filename += "/dejavu-lgc-sans-bold.ttf";
    else
      filename += "/dejavu-lgc-sans.ttf";

    if (bold) /* config file overrides built-in defaults */
      {
        if (cfg->font_bold() != "")
          filename = cfg->font_bold();
      }
    else
      {
        if (cfg->font() != "")
          filename = cfg->font();
      }

    if (FT_New_Face (ft_library, filename.c_str(), 0, out_face) == 0)
      {
        sm_debug ("found font %s\n", filename.c_str());
        return true;
      }
    else
      {
        sm_debug ("error loading font %s\n", filename.c_str());
        return false;
      }
  }
public:
  static TextRenderer *
  the()
  {
    static TextRenderer *instance = new TextRenderer(); /* TODO: cleanup */
    return instance;
  }
  TextRenderer() :
    cfg (new Config())
  {
    ft_init_ok = (FT_Init_FreeType (&ft_library) == 0);

    if (ft_init_ok)
      {
        load_font (&face,      false);  /* normal */
        load_font (&face_bold, true);   /* bold */
      }
  }

  FT_Face face {};
  FT_Face face_bold {};
  std::map<std::pair<int, bool>, std::map<FT_UInt, Glyph *>> glyph_caches;

  std::map<FT_UInt, Glyph *>&
  get_glyph_cache (double size, bool bold)
  {
    return glyph_caches[std::make_pair (lrint (size * 64), bold)];
  }

  size_t
  estimate_cache_size()
  {
    size_t size = 0;
    for (auto& cache : glyph_caches)
      {
        for (auto& cache_entry : cache.second)
          {
            Glyph *glyph = cache_entry.second;
            size += sizeof (Glyph) + glyph->bitmap.size();
          }
      }
    return size;
  }

  void
  clear_cache()
  {
    glyph_caches.clear();
  }
};


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

    FT_Face face = bold ? TextRenderer::the()->face_bold : TextRenderer::the()->face;

#if DEBUG_EXTENTS
    cairo_font_extents_t font_extents_cairo;
    cairo_font_extents (cr, &font_extents_cairo);

    cairo_text_extents_t extents_cairo;
    cairo_text_extents (cr, text.c_str(), &extents_cairo);
#endif

    // Use simple matrix while font rendering
    cairo_save (cr);

    cairo_matrix_t mat;
    cairo_get_matrix (cr, &mat);

    double s = (mat.xx + mat.yy) / 2;

    // Set font size (pixels)
    FT_Set_Char_Size (face, 0, lrint (11 * 64 * s), 0, 0);
    std::map<FT_UInt, Glyph *>& glyph_cache = TextRenderer::the()->get_glyph_cache (11 * s, bold);

    // zero out scale/skew
    mat.xx = 1.0;
    mat.yy = 1.0;
    mat.xy = 0.0;
    mat.yx = 0.0;
    cairo_set_matrix (cr, &mat);

    auto text32 = to_utf32 (text);
    std::vector<Glyph *> glyphs;
    int bb_left = 0, bb_top = 0, bb_right = 0, bb_bottom = 0, bb_pen_x = 0;
    bool first = true;
    for (auto p : text32)
      {
        FT_UInt glyph_index = FT_Get_Char_Index (face, p);
        if (!glyph_cache[glyph_index])
          {
            Glyph *glyph = new Glyph();
            glyph_cache[glyph_index] = glyph;

            // Load and render glyph
            if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
                continue; // skip on error
            }
            FT_GlyphSlot g = face->glyph;
            FT_Bitmap *bmp = &g->bitmap;

            for (int y = 0; y < bmp->rows; y++)
              {
                for (int x = 0; x < bmp->width; x++)
                  glyph->bitmap.push_back (bmp->buffer[y * bmp->pitch + x]);
              }

            glyph->advance_x = g->advance.x;
            glyph->bitmap_left = g->bitmap_left;
            glyph->bitmap_top = g->bitmap_top;
            glyph->bitmap_width = bmp->width;
            glyph->bitmap_height = bmp->rows;
            // printf ("cache size: %zd\n", TextRenderer::the()->estimate_cache_size());
          }
        glyphs.push_back (glyph_cache[glyph_index]);
        auto glyph = glyphs.back();
        if (!glyph->bitmap.empty()) // only use visible glyphs to compute bounding box, assume left->right order
          {
            if (first)
              {
                bb_left = bb_pen_x + glyph->bitmap_left;
                bb_right = bb_pen_x + glyph->bitmap_left + glyph->bitmap_width;
                bb_top = glyph->bitmap_top;
                bb_bottom = glyph->bitmap_height - glyph->bitmap_top;
                first = false;
              }
            else
              {
                bb_right = bb_pen_x + glyph->bitmap_left + glyph->bitmap_width;
                bb_top = std::max (bb_top, glyph->bitmap_top);
                bb_bottom = std::max (bb_bottom, glyph->bitmap_height - glyph->bitmap_top);
              }
          }
        bb_pen_x += glyph->advance_x / 64;
      }

    int bb_width = bb_right - bb_left;
    int bb_height = bb_bottom + bb_top;

    TextExtents extents;
    extents.x_bearing = bb_left / s;
    extents.y_bearing = -bb_top / s;
    extents.width = bb_width / s;
    extents.height = bb_height / s;
    extents.x_advance = bb_pen_x / s;

    FontExtents font_extents;
    font_extents.ascent = face->size->metrics.ascender / 64.0 / s;
    font_extents.descent = -face->size->metrics.descender / 64.0 / s;
    font_extents.height = face->size->metrics.height / 64.0 / s;
    font_extents.max_x_advance = face->size->metrics.max_advance / 64.0 / s;

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
    cairo_surface_t *glyph_surface = cairo_image_surface_create (CAIRO_FORMAT_A8, bb_width, bb_height);

    int xx = 0;
    for (auto glyph : glyphs)
      {
        unsigned char *dst = cairo_image_surface_get_data (glyph_surface);
        auto dst_stride = cairo_image_surface_get_stride (glyph_surface);

        for (int y = 0; y < glyph->bitmap_height; y++)
          {
            memcpy (&dst[(y + (bb_top - glyph->bitmap_top)) * dst_stride + (glyph->bitmap_left - bb_left) + xx],
                    &glyph->bitmap[y * glyph->bitmap_width],
                    glyph->bitmap_width);
          }
        xx += glyph->advance_x / 64;
      }
    cairo_surface_mark_dirty (glyph_surface);

    double ux = (pen_x * s) + bb_left;
    double uy = (pen_y * s) - bb_top;

    // Snap to integer device pixels
    cairo_user_to_device (cr, &ux, &uy);
    ux = round (ux);
    uy = round (uy);
    cairo_device_to_user (cr, &ux, &uy);

    cairo_mask_surface (cr, glyph_surface, ux, uy);
    cairo_surface_destroy (glyph_surface);

    cairo_restore (cr);

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

