// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WINDOW_HH
#define SPECTMORPH_WINDOW_HH

#include "smwidget.hh"
#include "pugl/pugl.h"
#include <stdio.h>

namespace SpectMorph
{

struct Window : public Widget
{
  PuglView     *view;
  PuglCairoGL   cairo_gl;
  cairo_t      *cr;

  Window (int width, int height, PuglNativeWindow parent = 0);
  virtual ~Window();

  std::vector<Widget *> crawl_widgets();
  void on_display();
};

}

#endif
