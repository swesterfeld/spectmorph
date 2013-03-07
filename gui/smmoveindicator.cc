// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmoveindicator.hh"

#include <assert.h>

#include <QPainter>

using namespace SpectMorph;

MoveIndicator::MoveIndicator()
{
  m_active = false;
  setMinimumSize (QSize (0, 10));
  setMaximumSize (QSize (QWIDGETSIZE_MAX, 10));
}


void
MoveIndicator::set_active (bool active)
{
  m_active = active;
  update();
}

void
MoveIndicator::paintEvent (QPaintEvent * /* event */)
{
  if (m_active)
    {
      QPainter painter (this);
      painter.fillRect (rect(), QColor (0, 0, 200));
    }
}
