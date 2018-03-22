// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smdrawutils.hh"
#include "smwindow.hh"

using namespace SpectMorph;

using std::string;

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

