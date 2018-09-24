// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"
#include "smdrawutils.hh"
#include "smmath.hh"
#include "smmorphplan.hh"
#include "smmorphgridview.hh"

using namespace SpectMorph;

using std::min;

MorphGridWidget::MorphGridWidget (Widget *parent, MorphGrid *morph_grid, MorphGridView *morph_grid_view) :
  Widget (parent),
  morph_grid (morph_grid)
{
  connect (morph_grid_view->signal_grid_params_changed, this, &MorphGridWidget::on_grid_params_changed);
  connect (signal_grid_params_changed, this, &MorphGridWidget::on_grid_params_changed);
}

void
MorphGridWidget::draw (const DrawEvent& devent)
{
  cairo_t *cr = devent.cr;
  DrawUtils du (devent.cr);

  du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

  const double node_rect_width = 40;
  const double node_rect_height = 20;

  start_x = 10 + node_rect_width / 2;
  end_x = width - node_rect_width / 2 - 10;
  start_y = 10 + node_rect_height / 2;
  end_y = height - node_rect_height / 2 - 10;

  x_coord.resize (morph_grid->width());
  y_coord.resize (morph_grid->height());

  for (int x = 0; x < morph_grid->width(); x++)
    {
      if (morph_grid->width() > 1)
        {
          x_coord[x] = start_x + (end_x - start_x) * double (x) / (morph_grid->width() - 1);
        }
      else
        {
          x_coord[x] = (start_x + end_x) / 2;
        }
    }

  for (int y = 0; y < morph_grid->height(); y++)
    {
      if (morph_grid->height() > 1)
        {
          y_coord[y] = start_y + (end_y - start_y) * double (y) / (morph_grid->height() - 1);
        }
      else
        {
          y_coord[y] = (start_y + end_y) / 2;
        }
    }

  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          Color fill_color (0.5, 0.5, 0.5);
          if (x == morph_grid->selected_x() && y == morph_grid->selected_y())
            fill_color = fill_color.lighter();

          Color line_color (1.0, 1.0, 1.0);

          const double corner_radius = min (node_rect_width, node_rect_height) / 2;

          du.set_color (line_color);
          cairo_set_line_width (cr, 1);

          if (x > 0)
            {
              cairo_move_to (cr, x_coord[x - 1] + node_rect_width / 2, y_coord[y]);
              cairo_line_to (cr, x_coord[x]     - node_rect_width / 2, y_coord[y]);
              cairo_stroke (cr);
            }
          if (y > 0)
            {
              cairo_move_to (cr, x_coord[x], y_coord[y - 1] + node_rect_height / 2);
              cairo_line_to (cr, x_coord[x], y_coord[y]     - node_rect_height / 2);
              cairo_stroke (cr);
            }

          du.round_box (x_coord[x] - node_rect_width / 2, y_coord[y] - node_rect_height / 2,
                        node_rect_width, node_rect_height, 1, corner_radius, line_color, fill_color);
        }
    }

  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          if (x == morph_grid->selected_x() && y == morph_grid->selected_y())
            du.set_color (Color (0, 0, 0));
          else
            du.set_color (Color (1, 1, 1));

          MorphGridNode node = morph_grid->input_node (x, y);

          if (!node.op && node.smset == "")
            du.set_color (Color (0.8, 0, 0));

          du.text (morph_grid->input_node_label (x, y),
                   x_coord[x] - node_rect_width / 2, y_coord[y] - node_rect_height / 2,
                   node_rect_width, node_rect_height, TextAlign::CENTER);
        }
    }


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
MorphGridWidget::mouse_press (double mouse_x, double mouse_y)
{
  const double mx = start_x + (end_x - start_x) * (morph_grid->x_morphing() + 1) / 2.0;
  const double my = start_y + (end_y - start_y) * (morph_grid->y_morphing() + 1) / 2.0;

  double mdx = mx - mouse_x;
  double mdy = my - mouse_y;
  double mdist = sqrt (mdx * mdx + mdy * mdy);
  if (mdist < 11)
    {
      move_controller = true;
      motion (mouse_x, mouse_y);
    }
  int selected_x = -1;
  int selected_y = -1;
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          double delta_x = x_coord[x] - mouse_x;
          double delta_y = y_coord[y] - mouse_y;
          double dist = sqrt (delta_x * delta_x + delta_y * delta_y);
          if (dist < 11)
            {
              selected_x = x;
              selected_y = y;
            }
        }
    }
  morph_grid->set_selected_x (selected_x);
  morph_grid->set_selected_y (selected_y);
  signal_selection_changed();
  update();

  if (selected_x == -1 && selected_y == -1)
    {
      move_controller = true;
      motion (mouse_x, mouse_y);
    }
}

void
MorphGridWidget::motion (double x, double y)
{
  if (move_controller)
    {
      double dx = (x - start_x) / double (end_x - start_x) * 2.0 - 1.0;
      double dy = (y - start_y) / double (end_y - start_y) * 2.0 - 1.0;

      morph_grid->set_x_morphing (sm_bound (-1.0, dx, 1.0));
      morph_grid->set_y_morphing (sm_bound (-1.0, dy, 1.0));

      signal_grid_params_changed();
    }
}

void
MorphGridWidget::mouse_release (double x, double y)
{
  move_controller = false;
}

void
MorphGridWidget::on_grid_params_changed()
{
  if (morph_grid->selected_x() >= morph_grid->width())
    {
      morph_grid->set_selected_x (-1);
      signal_selection_changed();
    }
  if (morph_grid->selected_y() >= morph_grid->height())
    {
      morph_grid->set_selected_y (-1);
      signal_selection_changed();
    }
  update();
}
