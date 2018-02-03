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
    cairo_set_font_size (cr, 12.0);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    if (align == Align::LEFT)
      cairo_move_to (cr, 0, (height / 2) + extents.height / 2);
    else if (align == Align::CENTER)
      cairo_move_to (cr, (width / 2) - extents.width / 2, (height / 2) + extents.height / 2);
    else
      cairo_move_to (cr, width - extents.width, (height / 2) + extents.height / 2);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_show_text (cr, text.c_str());
  }
};

}

#endif
