// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"

#include <QPainter>

using namespace SpectMorph;

MorphGridWidget::MorphGridWidget()
{
  setMinimumSize (200, 200);
}

void
MorphGridWidget::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.fillRect (rect(), QColor (255, 255, 255));
}
