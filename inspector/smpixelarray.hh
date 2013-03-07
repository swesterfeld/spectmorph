// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PIXELARRAY_HH
#define SPECTMORPH_PIXELARRAY_HH

#include <vector>
#include <sys/types.h>

namespace SpectMorph {

struct PixelArray
{
  size_t width;
  size_t height;
  std::vector<int> pixels;

public:
  PixelArray();

  bool empty();
  void resize (size_t width, size_t height);
  void clear();
  int* get_pixels();
  size_t get_rowstride();
  size_t get_height();
  size_t get_width();
};

}

#endif
