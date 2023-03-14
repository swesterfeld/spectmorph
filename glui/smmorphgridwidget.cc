// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphgridwidget.hh"
#include "smdrawutils.hh"
#include "smmath.hh"
#include "smmorphplan.hh"
#include "smmorphgridview.hh"

using namespace SpectMorph;

using std::vector;
using std::min;

MorphGridWidget::MorphGridWidget (Widget *parent, MorphGrid *morph_grid, MorphGridView *morph_grid_view) :
  Widget (parent),
  morph_grid (morph_grid),
  prop_x_morphing (*morph_grid->property (MorphGrid::P_X_MORPHING)),
  prop_y_morphing (*morph_grid->property (MorphGrid::P_Y_MORPHING))
{
  connect (morph_grid_view->signal_grid_params_changed, this, &MorphGridWidget::on_grid_params_changed);
  connect (signal_grid_params_changed, this, &MorphGridWidget::on_grid_params_changed);
}

Point
MorphGridWidget::prop_to_pixel (double x, double y)
{
  return Point (start_x + (end_x - start_x) * (x + 1) / 2.0,
                start_y + (end_y - start_y) * (y + 1) / 2.0);
}

void
MorphGridWidget::draw (const DrawEvent& devent)
{
  cairo_t *cr = devent.cr;
  DrawUtils du (devent.cr);

  du.round_box (0, 0, width(), height(), 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

  const double node_rect_width = 40;
  const double node_rect_height = 20;

  start_x = 10 + node_rect_width / 2;
  end_x = width() - node_rect_width / 2 - 10;
  start_y = 10 + node_rect_height / 2;
  end_y = height() - node_rect_height / 2 - 10;

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

  double spr_w, spr_h;
  window()->get_sprite_size (spr_w, spr_h);

  for (size_t v = 0; v < x_voice_values.size(); v++)
    {
      const auto p = prop_to_pixel (x_voice_values[v], y_voice_values[v]);

      window()->draw_sprite (this, p.x() - spr_w / 2, p.y() - spr_h / 2);
    }

  if (prop_x_morphing.modulation_list()->main_control_type() == MorphOperator::CONTROL_GUI
  ||  prop_y_morphing.modulation_list()->main_control_type() == MorphOperator::CONTROL_GUI)
    {
      const auto pm = prop_to_pixel (morph_grid->x_morphing(), morph_grid->y_morphing());

      cairo_set_source_rgb (cr, 0.5, 0.5, 1.0);
      cairo_set_line_width (cr, 3);

      cairo_move_to (cr, pm.x() - 10, pm.y() - 10);
      cairo_line_to (cr, pm.x() + 10, pm.y() + 10);
      cairo_move_to (cr, pm.x() - 10, pm.y() + 10);
      cairo_line_to (cr, pm.x() + 10, pm.y() - 10);
      cairo_stroke (cr);
    }
}

void
MorphGridWidget::mouse_press (const MouseEvent& event)
{
  if (event.button != LEFT_BUTTON)
    return;

  const auto pm = prop_to_pixel (morph_grid->x_morphing(), morph_grid->y_morphing());

  double mdx = pm.x() - event.x;
  double mdy = pm.y() - event.y;
  double mdist = sqrt (mdx * mdx + mdy * mdy);
  if (mdist < 11)
    {
      move_controller = true;
      mouse_move (event);
    }
  int selected_x = -1;
  int selected_y = -1;
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          double delta_x = x_coord[x] - event.x;
          double delta_y = y_coord[y] - event.y;
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
      mouse_move (event);
    }
}

void
MorphGridWidget::mouse_move (const MouseEvent& event)
{
  if (move_controller)
    {
      double dx = (event.x - start_x) / double (end_x - start_x) * 2.0 - 1.0;
      double dy = (event.y - start_y) / double (end_y - start_y) * 2.0 - 1.0;

      morph_grid->set_x_morphing (sm_bound (-1.0, dx, 1.0));
      morph_grid->set_y_morphing (sm_bound (-1.0, dy, 1.0));

      signal_grid_params_changed();
    }
}

void
MorphGridWidget::mouse_release (const MouseEvent& event)
{
  if (event.button == LEFT_BUTTON)
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

void
MorphGridWidget::redraw_voices()
{
  double spr_w, spr_h;
  window()->get_sprite_size (spr_w, spr_h);

  for (size_t v = 0; v < x_voice_values.size(); v++)
    {
      const auto p = prop_to_pixel (x_voice_values[v], y_voice_values[v]);

      update (p.x() - spr_w / 2, p.y() - spr_h / 2, spr_w, spr_h, UPDATE_LOCAL);
    }
}

void
MorphGridWidget::on_voice_status_changed (VoiceStatus *voice_status)
{
  redraw_voices();
  x_voice_values = voice_status->get_values (prop_x_morphing);
  y_voice_values = voice_status->get_values (prop_y_morphing);

  if (morph_grid->width() == 1)
    std::fill (x_voice_values.begin(), x_voice_values.end(), 0);
  if (morph_grid->height() == 1)
    std::fill (y_voice_values.begin(), y_voice_values.end(), 0);

  redraw_voices();
}
