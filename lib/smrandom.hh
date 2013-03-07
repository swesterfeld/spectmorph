// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_RANDOM_HH
#define SPECTMORPH_RANDOM_HH

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

namespace SpectMorph
{

class Random
{
  char                state[32];
  struct random_data  buf;
public:
  Random();

  void set_seed (int seed);

  inline double
  random_double_range (double begin, double end)
  {
    int32_t r;
    random_r (&buf, &r);

    const double scale = 1.0 / (double (RAND_MAX) + 1.0);
    return r * scale * (end - begin) + begin;
  }
  inline int32_t
  random_int32()
  {
    int32_t r;
    random_r (&buf, &r);
    return r;
  }
  inline void
  random_block (size_t   n_values,
                guint32 *values)
  {
    guint64 random_data = 0;
    int     random_bits = 0;

    while (n_values--)
      {
        while (random_bits < 32)
          {
            random_data ^= guint64 (random_int32()) << random_bits;
            random_bits += 31;
          }
        *values++ = random_data;
        random_data >>= 32;
        random_bits -= 32;
      }
  }
};

}

#endif
