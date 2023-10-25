// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smdrawutils.hh"
#include "smwindow.hh"
#include "smconfig.hh"
#include "smmain.hh"

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

namespace SpectMorph {

struct GlobalFontData {
  map<string, std::pair<bool, FT_Face>> ft_map;
  std::unique_ptr<Config>               cfg;
  bool                                  ft_init_ok = false;
  FT_Library                             ft_library;

  GlobalFontData() :
    cfg (new Config())
  {
    ft_init_ok = (FT_Init_FreeType (&ft_library) == 0);
  }
  ~GlobalFontData()
  {
    if (ft_init_ok)
      FT_Done_FreeType (ft_library);
  }
};

}

void
DrawUtils::select_font_face (bool bold)
{
  static GlobalFontData *global_font_data = nullptr;
  if (!global_font_data)
    {
      global_font_data = new GlobalFontData();
      sm_global_free_func ([]()
        {
          delete global_font_data;
          global_font_data = nullptr;
        });
    }

  string filename = sm_get_install_dir (INSTALL_DIR_FONTS);
  if (bold)
    filename += "/dejavu-lgc-sans-bold.ttf";
  else
    filename += "/dejavu-lgc-sans.ttf";

  if (bold) /* config file overrides built-in defaults */
    {
      if (global_font_data->cfg->font_bold() != "")
        filename = global_font_data->cfg->font_bold();
    }
  else
    {
      if (global_font_data->cfg->font() != "")
        filename = global_font_data->cfg->font();
    }

  if (global_font_data->ft_map.find (filename) == global_font_data->ft_map.end())
    {
      FT_Face face;
      if (global_font_data->ft_init_ok)
        {
          /* only load each font once */
          if (FT_New_Face (global_font_data->ft_library, filename.c_str(), 0, &face) == 0)
            {
              global_font_data->ft_map[filename] = std::make_pair (true, face);
              sm_debug ("found font %s\n", filename.c_str());
            }
          else
            {
              global_font_data->ft_map[filename].first = false; // don't try again
              sm_debug ("error loading font %s\n", filename.c_str());
            }
        }
    }
  auto ft_map_entry = global_font_data->ft_map[filename];
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
