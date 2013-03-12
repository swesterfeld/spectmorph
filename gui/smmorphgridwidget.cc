// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"
#include "smmorphplan.hh"

#include <QPainter>
#include <QMouseEvent>

using namespace SpectMorph;

using std::vector;

MorphGridWidget::MorphGridWidget (MorphGrid *morph_grid) :
  morph_grid (morph_grid)
{
  setMinimumSize (200, 200);

  connect (morph_grid->morph_plan(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
}

void
MorphGridWidget::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.fillRect (rect(), QColor (100, 100, 100));

  painter.setBrush (QColor (255, 255, 255, 128));
  painter.setPen (QPen (QColor (200, 200, 200), 2));

  x_coord.resize (morph_grid->width());
  y_coord.resize (morph_grid->height());

  int start_x = 20;
  int end_x = width() - 20;
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

  int start_y = 20;
  int end_y = height() - 20;
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
          if (x == morph_grid->selected_x() && y == morph_grid->selected_y())
            painter.setBrush (QColor (255, 255, 255, 230));
          else
            painter.setBrush (QColor (255, 255, 255, 128));

          painter.drawEllipse (QRect (x_coord[x] - 10, y_coord[y] - 10, 20, 20));

          if (x > 0)
            painter.drawLine (x_coord[x - 1] + 10, y_coord[y], x_coord[x] - 10, y_coord[y]);
          if (y > 0)
            painter.drawLine (x_coord[x], y_coord[y - 1] + 10, x_coord[x], y_coord[y] - 10);
        }
    }
  painter.setPen (QPen (QColor (200, 0, 0), 2));
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          if (!morph_grid->input_op (x, y))
            {
              painter.drawLine (x_coord[x] - 10, y_coord[y] - 10, x_coord[x] + 10, y_coord[y] + 10);
              painter.drawLine (x_coord[x] + 10, y_coord[y] - 10, x_coord[x] - 10, y_coord[y] + 10);
            }
        }
    }
}

void
MorphGridWidget::on_plan_changed()
{
  if (morph_grid->selected_x() >= morph_grid->width())
    {
      morph_grid->set_selected_x (-1);
      emit selection_changed();
    }
  if (morph_grid->selected_y() >= morph_grid->height())
    {
      morph_grid->set_selected_y (-1);
      emit selection_changed();
    }
  update();
}

void
MorphGridWidget::mousePressEvent (QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
    {
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
      emit selection_changed();
      update();
    }
}

void
MorphGridWidget::mouseMoveEvent (QMouseEvent *event)
{
}

void
MorphGridWidget::mouseReleaseEvent (QMouseEvent *event)
{
}
