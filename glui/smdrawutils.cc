// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smdrawutils.hh"
#include "smwindow.hh"

#include <map>

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_TYPE1_TABLES_H

using namespace SpectMorph;

using std::string;
using std::map;

double
DrawUtils::static_text_width (Window *window, const string& text)
{
  cairo_surface_t *dummy = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 0, 0);
  cairo_t *cr = cairo_create (dummy);

  double global_scale = window ? window->gui_scaling() : 1.0;
  cairo_scale (cr, global_scale, global_scale);

  DrawUtils du (cr);
  double w = du.text_width (text);

  cairo_surface_destroy (dummy);
  cairo_destroy (cr);
  return w;
}

cairo_text_extents_t
DrawUtils::static_text_extents (Window *window, const string& text)
{
  cairo_surface_t *dummy = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 0, 0);
  cairo_t *cr = cairo_create (dummy);

  double global_scale = window ? window->gui_scaling() : 1.0;
  cairo_scale (cr, global_scale, global_scale);

  DrawUtils du (cr);
  auto extents = du.text_extents (text);

  cairo_surface_destroy (dummy);
  cairo_destroy (cr);
  return extents;
}

void
DrawUtils::select_font_face (bool bold)
{
  static map<string, std::pair<bool, FT_Face>> ft_map;

  string filename = sm_get_install_dir (INSTALL_DIR_FONTS);
  if (bold)
    filename += "/Vera-Bold.ttf";
  else
    filename += "/Vera.ttf";

  if (ft_map.find (filename) == ft_map.end())
    {
      FT_Face face;
      FT_Library value;

      if (FT_Init_FreeType (&value) == 0)
        {
          /* only load each font once */
          if (FT_New_Face (value, filename.c_str(), 0, &face) == 0)
            {
              ft_map[filename] = std::make_pair (true, face);
              sm_debug ("found font %s\n", filename.c_str());
            }
          else
            {
              ft_map[filename].first = false; // don't try again
              sm_debug ("error loading font %s\n", filename.c_str());
            }
        }
    }
  auto ft_map_entry = ft_map[filename];
  if (ft_map_entry.first)
    {
      cairo_font_face_t *ct = cairo_ft_font_face_create_for_ft_face (ft_map_entry.second, 0);
      cairo_set_font_face (cr, ct);
    }
  else
    {
      cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    }
}
