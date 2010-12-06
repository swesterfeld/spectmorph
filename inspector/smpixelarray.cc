/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smpixelarray.hh"

using namespace SpectMorph;

PixelArray::PixelArray()
{
  width = 0;
  height = 0;
}

void
PixelArray::resize (size_t width, size_t height)
{
  this->width = width;
  this->height = height;

  pixels.clear();
  pixels.resize (width * height);
}

void
PixelArray::clear()
{
  resize (0, 0);
}

bool
PixelArray::empty()
{
  return (width == 0) && (height == 0);
}

unsigned char *
PixelArray::get_pixels()
{
  return &pixels[0];
}

size_t
PixelArray::get_rowstride()
{
  return width;
}

size_t
PixelArray::get_width()
{
  return width;
}

size_t
PixelArray::get_height()
{
  return height;
}


