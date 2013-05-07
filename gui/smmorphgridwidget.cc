// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"
#include "smmorphplan.hh"

#include <QPainter>
#include <QMouseEvent>

using namespace SpectMorph;

using std::vector;
using std::max;
using std::min;

MorphGridWidget::MorphGridWidget (MorphGrid *morph_grid) :
  morph_grid (morph_grid)
{
  // debug: label_font.setPixelSize (24);
  move_controller = false;
  update_size();
  connect (morph_grid->morph_plan(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
}

void
MorphGridWidget::update_size()
{
  int node_rect_width;
  int node_rect_height;
  compute_node_rect (&node_rect_width, &node_rect_height);

  int base_size;
  base_size = pow (1.2, morph_grid->zoom()) * max (node_rect_width, node_rect_height);
  setFixedSize ((morph_grid->width() - 1) * base_size + 20 + node_rect_width,
                (morph_grid->height() - 1) * base_size + 20 + node_rect_height);
}

void
MorphGridWidget::compute_node_rect (int *width, int *height)
{
  QFontMetrics font_metrics (label_font);

  int node_rect_width = 0, node_rect_height = 0;
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          QRect r = font_metrics.boundingRect (morph_grid->input_node_label (x, y).c_str());
          node_rect_width = max (node_rect_width, r.width());
          node_rect_height = max (node_rect_height, r.height());
        }
    }
  node_rect_height += 5; // space around label
  node_rect_width += 20; // round corners

  if (width)
    *width = node_rect_width;

  if (height)
    *height = node_rect_height;
}

void
MorphGridWidget::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.fillRect (rect(), QColor (100, 100, 100));

  painter.setBrush (QColor (255, 255, 255, 128));
  painter.setPen (QPen (QColor (200, 200, 200), 2));
  painter.setFont (label_font);

  x_coord.resize (morph_grid->width());
  y_coord.resize (morph_grid->height());

  int node_rect_width, node_rect_height;
  compute_node_rect (&node_rect_width, &node_rect_height);

  const int start_x = 10 + node_rect_width / 2;
  const int end_x = width() - node_rect_width / 2 - 10;
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

  const int start_y = 10 + node_rect_height / 2;
  const int end_y = height() - node_rect_height / 2 - 10;
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

  const int corner_radius = min (node_rect_width, node_rect_height) / 2;
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          if (x == morph_grid->selected_x() && y == morph_grid->selected_y())
            painter.setBrush (QColor (255, 255, 255, 230));
          else
            painter.setBrush (QColor (255, 255, 255, 128));

          painter.drawRoundedRect (QRect (x_coord[x] - node_rect_width / 2,
                                          y_coord[y] - node_rect_height / 2,
                                          node_rect_width, node_rect_height), corner_radius, corner_radius);

          if (x > 0)
            painter.drawLine (x_coord[x - 1] + node_rect_width / 2, y_coord[y],
                              x_coord[x]     - node_rect_width / 2, y_coord[y]);
          if (y > 0)
            painter.drawLine (x_coord[x], y_coord[y - 1] + node_rect_height / 2,
                              x_coord[x], y_coord[y]     - node_rect_height / 2);
        }
    }

  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          if (x == morph_grid->selected_x() && y == morph_grid->selected_y())
            painter.setPen (QColor (0, 0, 0));
          else
            painter.setPen (QColor (255, 255, 255));

          MorphGridNode node = morph_grid->input_node (x, y);

          if (!node.op && node.smset == "")
            painter.setPen (QColor (200, 0, 0));

          QRect rect (x_coord[x] - node_rect_width / 2, y_coord[y] - node_rect_height / 2,
                      node_rect_width, node_rect_height);
          painter.drawText (rect, Qt::AlignCenter, morph_grid->input_node_label (x, y).c_str());
        }
    }
  const int mx = start_x + (end_x - start_x) * (morph_grid->x_morphing() + 1) / 2.0;
  const int my = start_y + (end_y - start_y) * (morph_grid->y_morphing() + 1) / 2.0;

  painter.setPen (QPen (QColor (130, 130, 255), 3));
  painter.drawLine (mx - 10, my - 10, mx + 10, my + 10);
  painter.drawLine (mx - 10, my + 10, mx + 10, my - 10);
}

void
MorphGridWidget::on_plan_changed()
{
  if (morph_grid->selected_x() >= morph_grid->width())
    {
      morph_grid->set_selected_x (-1);
      Q_EMIT selection_changed();
    }
  if (morph_grid->selected_y() >= morph_grid->height())
    {
      morph_grid->set_selected_y (-1);
      Q_EMIT selection_changed();
    }
  update_size();
  update();
}

void
MorphGridWidget::mousePressEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      int start_x = 20;
      int end_x = width() - 20;
      int start_y = 20;
      int end_y = height() - 20;

      int mx = start_x + (end_x - start_x) * (morph_grid->x_morphing() + 1) / 2.0;
      int my = start_y + (end_y - start_y) * (morph_grid->y_morphing() + 1) / 2.0;

      double mdx = mx - event->pos().x();
      double mdy = my - event->pos().y();
      double mdist = sqrt (mdx * mdx + mdy * mdy);
      if (mdist < 11)
        {
          move_controller = true;
          setCursor (Qt::SizeAllCursor);
          return;
        }

      int selected_x = -1;
      int selected_y = -1;
      for (int x = 0; x < morph_grid->width(); x++)
        {
          for (int y = 0; y < morph_grid->height(); y++)
            {
              double delta_x = x_coord[x] - event->pos().x();
              double delta_y = y_coord[y] - event->pos().y();
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
      Q_EMIT selection_changed();
      update();
    }
}

void
MorphGridWidget::mouseMoveEvent (QMouseEvent *event)
{
  if (move_controller)
    {
      int start_x = 20;
      int end_x = width() - 20;
      int start_y = 20;
      int end_y = height() - 20;
      double dx = (event->pos().x() - start_x) / double (end_x - start_x) * 2.0 - 1.0;
      double dy = (event->pos().y() - start_y) / double (end_y - start_y) * 2.0 - 1.0;
      morph_grid->set_x_morphing (qBound (-1.0, dx, 1.0));
      morph_grid->set_y_morphing (qBound (-1.0, dy, 1.0));
    }
}

void
MorphGridWidget::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
      move_controller = false;
      unsetCursor();
    }
}
