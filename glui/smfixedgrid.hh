// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_FIXED_GRID_HH
#define SPECTMORPH_FIXED_GRID_HH

namespace SpectMorph
{

struct FixedGrid
{
  double dx = 0;
  double dy = 0;

  FixedGrid (Widget *relative_to = nullptr)
  {
    if (relative_to)
      {
        dx = relative_to->x() / 8;
        dy = relative_to->y() / 8;
      }
  }
  void add_widget (Widget *w, double x, double y, double width, double height)
  {
    w->set_x ((x + dx) * 8);
    w->set_y ((y + dy) * 8);
    w->set_width (width * 8);
    w->set_height (height * 8);
  }
};

}

#endif
