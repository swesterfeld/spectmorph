/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
