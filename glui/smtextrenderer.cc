// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smtextrenderer.hh"

#include <fontconfig/fontconfig.h>

#include <math.h>

using namespace SpectMorph;

using std::string;
using std::vector;

// Helper: find a font file for a family + style string using fontconfig
static string
find_font_file (const string& family, bool bold, bool exact)
{
  string desc = family + ":style=" + (bold ? "Bold" : "Regular");

  if (!FcInit())
    {
      fprintf (stderr, "fontconfig init failed\n");
      return "";
    }
  FcPattern *pattern = FcNameParse ((const FcChar8 *)desc.c_str());
  if (!pattern)
    {
      fprintf (stderr, "failed to parse fontconfig pattern: %s\n", desc.c_str());
      return "";
    }

  FcConfigSubstitute (NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute (pattern);

  FcResult result;
  FcPattern *match = FcFontMatch (NULL, pattern, &result);
  FcPatternDestroy (pattern);

  if (!match)
    {
      fprintf(stderr, "no match for fontconfig pattern: %s\n", desc.c_str());
      return "";
    }
  // if asked for an exact match, fail if familty doesn't match requested family
  if (exact)
    {
      FcChar8 *match_family = NULL;
      if (FcPatternGetString (match, FC_FAMILY, 0, &match_family) == FcResultMatch)
        {
          string match_family_str = (char *)match_family;
          if (match_family_str != family)
            {
              FcPatternDestroy (match);
              return "";
            }
        }
    }


  FcChar8 *file = NULL;
  if (FcPatternGetString (match, FC_FILE, 0, &file) != FcResultMatch)
    {
      fprintf (stderr, "no fontconfig file property for: %s\n", desc.c_str());
      FcPatternDestroy (match);
      return "";
    }

  // Duplicate the filename so we can free the pattern
  string filename = (const char *)file;
  FcPatternDestroy (match);

  return filename;
}

bool
TextRenderer::load_font (FT_Face *out_face, bool bold) const
{
  string filename = sm_get_install_dir (INSTALL_DIR_FONTS);
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
  sm_debug ("error loading font %s\n", filename.c_str());

  filename = find_font_file ("DejaVu Sans", bold, /* exact */ true);
  if (filename == "")
    filename = find_font_file ("sans-serif", bold, /* exact */ false);

  if (FT_New_Face (ft_library, filename.c_str(), 0, out_face) == 0)
    {
      sm_debug ("found font using fontconfig %s\n", filename.c_str());
      return true;
    }
  return false;
}

std::unordered_map<char32_t, std::unique_ptr<TextRenderer::Glyph>>&
TextRenderer::get_glyph_cache (double size, bool bold)
{
  return glyph_caches[std::make_pair (lrint (size * 64), bold)];
}

TextRenderer::TextRenderer() :
  cfg (new Config())
{
  ft_init_ok = (FT_Init_FreeType (&ft_library) == 0);

  if (ft_init_ok)
    {
      load_font (&face_normal, false);  /* normal */
      load_font (&face_bold,   true);   /* bold */
    }
}

TextRenderer::~TextRenderer()
{
  /* FT_Done_...() functions are safe to call on nullptr */
  FT_Done_Face (face_normal);
  FT_Done_Face (face_bold);
  FT_Done_FreeType (ft_library);
}

size_t
TextRenderer::estimate_cache_size()
{
  std::lock_guard lg (mutex);

  size_t size = 0;
  for (auto& cache : glyph_caches)
    {
      for (auto& cache_entry : cache.second)
        {
          auto& glyph = cache_entry.second;
          size += sizeof (Glyph) + glyph->bitmap.size();
        }
    }
  return size;
}

void
TextRenderer::clear_cache()
{
  std::lock_guard lg (mutex);

  glyph_caches.clear();
}

cairo_surface_t *
TextRenderer::text_to_surface (double ui_scaling, bool bold, const string& text, FontExtents *font_extents, TextExtents *extents, Mode mode)
{
  std::lock_guard lg (mutex);

  FT_Face face = bold ? face_bold : face_normal;
  if (!face)
    {
      if (font_extents)
        *font_extents = FontExtents();
      if (extents)
        *extents = TextExtents();

      if (mode == Mode::EXTENTS_ONLY)
        return nullptr;
      else
        return cairo_image_surface_create (CAIRO_FORMAT_A8, 0, 0);
    }

  // Set font size (pixels)
  constexpr double font_size = 11.0;
  FT_Set_Char_Size (face, 0, lrint (font_size * 64 * ui_scaling), 0, 0);

  auto& glyph_cache = get_glyph_cache (font_size * ui_scaling, bold);
  vector<Glyph *> glyphs;
  auto text32 = to_utf32 (text);

  glyphs.reserve (text32.size());
  for (auto char32 : text32)
    {
      auto& cached_glyph = glyph_cache[char32];
      if (!cached_glyph)
        {
          cached_glyph = std::make_unique<Glyph>();

          FT_UInt glyph_index = FT_Get_Char_Index (face, char32);
          // Load and render glyph
          //  - on error: result is an invisible 0x0 glyph
          FT_Error error = FT_Load_Glyph (face, glyph_index, FT_LOAD_RENDER);
          if (!error)
            {
              FT_GlyphSlot g = face->glyph;
              FT_Bitmap *bmp = &g->bitmap;

              cached_glyph->bitmap.reserve (bmp->width * bmp->rows);
              for (uint y = 0; y < bmp->rows; y++)
                {
                  for (uint x = 0; x < bmp->width; x++)
                    cached_glyph->bitmap.push_back (bmp->buffer[y * bmp->pitch + x]);
                }

              cached_glyph->advance_x = g->advance.x >> 6;
              cached_glyph->bitmap_left = g->bitmap_left;
              cached_glyph->bitmap_top = g->bitmap_top;
              cached_glyph->bitmap_width = bmp->width;
              cached_glyph->bitmap_height = bmp->rows;
           }
        }
      glyphs.push_back (cached_glyph.get());
    }
  int bb_left = 0, bb_top = 0, bb_right = 0, bb_bottom = 0, bb_pen_x = 0;
  bool first = true;
  for (auto glyph : glyphs)
    {
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
      bb_pen_x += glyph->advance_x;
    }

  int bb_width = bb_right - bb_left;
  int bb_height = bb_bottom + bb_top;

  if (extents)
    {
      extents->x_bearing = bb_left / ui_scaling;
      extents->y_bearing = -bb_top / ui_scaling;
      extents->width = bb_width / ui_scaling;
      extents->height = bb_height / ui_scaling;
      extents->x_advance = bb_pen_x / ui_scaling;
    }
  if (font_extents)
    {
      font_extents->ascent = face->size->metrics.ascender / 64.0 / ui_scaling;
      font_extents->descent = -face->size->metrics.descender / 64.0 / ui_scaling;
      font_extents->height = face->size->metrics.height / 64.0 / ui_scaling;
      font_extents->max_x_advance = face->size->metrics.max_advance / 64.0 / ui_scaling;
    }

  if (mode == Mode::EXTENTS_ONLY)
    return nullptr;

  cairo_surface_t *text_surface = cairo_image_surface_create (CAIRO_FORMAT_A8, bb_width, bb_height);

  int xx = 0;
  for (auto glyph : glyphs)
    {
      unsigned char *dst = cairo_image_surface_get_data (text_surface);
      auto dst_stride = cairo_image_surface_get_stride (text_surface);
      unsigned char *src = glyph->bitmap.data();
      auto src_width = glyph->bitmap_width;

      dst += (bb_top - glyph->bitmap_top) * dst_stride + (glyph->bitmap_left - bb_left) + xx;

      for (int y = 0; y < glyph->bitmap_height; y++)
        {
          memcpy (dst, src, src_width);
          dst += dst_stride;
          src += src_width;
        }
      xx += glyph->advance_x;
    }
  cairo_surface_mark_dirty (text_surface);

  return text_surface;
}

