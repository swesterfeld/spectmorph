// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"
#include "smdrawutils.hh"
#include "smmath.hh"

using namespace SpectMorph;

MorphGridWidget::MorphGridWidget (Widget *parent, MorphGrid *morph_grid) :
  Widget (parent),
  morph_grid (morph_grid)
{
}

void
MorphGridWidget::draw (cairo_t *cr)
{
  DrawUtils du (cr);

  du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

  const double start_x = 20;
  const double end_x = height - 20;
  const double start_y = 20;
  const double end_y = width - 20;

  const double mx = start_x + (end_x - start_x) * (morph_grid->x_morphing() + 1) / 2.0;
  const double my = start_y + (end_y - start_y) * (morph_grid->y_morphing() + 1) / 2.0;

  cairo_set_source_rgb (cr, 0.5, 0.5, 1.0);
  cairo_set_line_width (cr, 3);

  cairo_move_to (cr, mx - 10, my - 10);
  cairo_line_to (cr, mx + 10, my + 10);
  cairo_move_to (cr, mx - 10, my + 10);
  cairo_line_to (cr, mx + 10, my - 10);
  cairo_stroke (cr);
}

void
MorphGridWidget::mouse_press (double x, double y)
{
#if 0
  const double start_x = 20;
  const double end_x = height - 20;
  const double start_y = 20;
  const double end_y = width - 20;

  const double mx = start_x + (end_x - start_x) * (morph_grid->x_morphing() + 1) / 2.0;
  const double my = start_y + (end_y - start_y) * (morph_grid->y_morphing() + 1) / 2.0;

  double mdx = mx - x;
  double mdy = my - y;
  double mdist = sqrt (mdx * mdx + mdy * mdy);
  if (mdist < 11)
#endif
    {
      move_controller = true;
      motion (x, y);
    }
}

void
MorphGridWidget::motion (double x, double y)
{
  if (move_controller)
    {
      const double start_x = 20;
      const double end_x = height - 20;
      const double start_y = 20;
      const double end_y = width - 20;

      double dx = (x - start_x) / double (end_x - start_x) * 2.0 - 1.0;
      double dy = (y - start_y) / double (end_y - start_y) * 2.0 - 1.0;

      morph_grid->set_x_morphing (sm_bound (-1.0, dx, 1.0));
      morph_grid->set_y_morphing (sm_bound (-1.0, dy, 1.0));
    }
}

void
MorphGridWidget::mouse_release (double x, double y)
{
  move_controller = false;
}
