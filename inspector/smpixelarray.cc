// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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

int *
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


