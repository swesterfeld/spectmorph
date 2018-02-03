// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LABEL_HH
#define SPECTMORPH_LABEL_HH

namespace SpectMorph
{

struct Label : public Widget
{
  std::string text;
  enum class Align {
    LEFT,
    CENTER,
    RIGHT
  } align;

  Label (Widget *parent, double x, double y, double w, double h, const std::string& text) :
    Widget (parent, x, y, w, h),
    text (text)
  {
    align = Align::LEFT;
  }
  void
  draw (cairo_t *cr) override
  {
    //debug_fill (cr);

    // draw label
    cairo_set_font_size (cr, 11.0);

    cairo_font_extents_t font_extents;
    cairo_font_extents (cr, &font_extents);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    double fy = height / 2 - font_extents.descent + font_extents.height / 2;
    if (align == Align::LEFT)
      cairo_move_to (cr, 0, fy);
    else if (align == Align::CENTER)
      cairo_move_to (cr, (width / 2) - extents.x_bearing - extents.width / 2, fy);
    else
      cairo_move_to (cr, width - extents.x_bearing - extents.width, fy);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_show_text (cr, text.c_str());
  }
};

}

#endif
