// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smconfig.hh"
#include <map>
#include <unordered_map>
#include <cairo.h>
#include <memory>
#include <mutex>
#include <ft2build.h>
#include FT_FREETYPE_H
#define DEBUG_EXTENTS 0

namespace SpectMorph
{

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

class TextRenderer {
  std::mutex                            mutex;
  std::unique_ptr<Config>               cfg;
  bool                                  ft_init_ok = false;
  FT_Library                            ft_library {};
  FT_Face                               face_normal {};
  FT_Face                               face_bold {};

  struct Glyph
  {
    int bitmap_left;
    int bitmap_top;
    int bitmap_width;
    int bitmap_height;
    std::vector<uint8_t> bitmap;
    int advance_x;
  };

  std::map<std::pair<int, bool>, std::unordered_map<char32_t, std::unique_ptr<Glyph>>> glyph_caches;

  bool load_font (FT_Face *out_face, bool bold) const;
  std::unordered_map<char32_t, std::unique_ptr<Glyph>>& get_glyph_cache (double size, bool bold);

public:
  static TextRenderer *the(); /* TODO: cleanup */

  TextRenderer();

  size_t estimate_cache_size();
  void clear_cache();

  enum class Mode {
    EXTENTS_ONLY,
    RENDER_SURFACE
  };
  cairo_surface_t *text_to_surface (double ui_scaling, bool bold, const std::string& text, FontExtents *font_extents, TextExtents *extents, Mode mode);
};

}
