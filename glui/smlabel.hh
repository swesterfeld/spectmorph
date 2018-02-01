// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LABEL_HH
#define SPECTMORPH_LABEL_HH

namespace SpectMorph
{

struct Label : public Widget
{
  std::string text;

  Label (double x, double y, double w, double h, const std::string& text) :
    Widget (x, y, w, h),
    text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    //debug_fill (cr);

    // draw label
    cairo_set_font_size (cr, 24.0);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    cairo_move_to (cr, (width / 2) - extents.width / 2, (height / 2) + extents.height / 2);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_show_text (cr, text.c_str());
  }
};

}

#endif
