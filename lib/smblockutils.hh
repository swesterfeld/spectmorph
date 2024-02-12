// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_BLOCK_UTILS_HH
#define SPECTMORPH_BLOCK_UTILS_HH

#include <glib.h>

namespace SpectMorph
{

/* Block utils */

class Block
{
public:
  // auto vectorization for simple cases is quite good these days,
  // so we don't provide an SSE intrinsics implementation for the block functions
  //
  static void
  mul (guint n_values, float *__restrict__ ovalues, const float *__restrict__ ivalues)
  {
    for (guint i = 0; i < n_values; i++)
      ovalues[i] *= ivalues[i];
  }
  static void
  add (guint n_values, float *__restrict__ ovalues, const float *__restrict__ ivalues)
  {
    for (guint i = 0; i < n_values; i++)
      ovalues[i] += ivalues[i];
  }
  static void
  sum2 (guint n_values, float *__restrict__ ovalues, const float *__restrict__ ivalues, const float *__restrict__ ivalues2)
  {
    for (guint i = 0; i < n_values; i++)
      ovalues[i] = ivalues[i] + ivalues2[i];
  }
  static void  range  (guint           n_values,
                       const float    *ivalues,
                       float&          min_value,
                       float&          max_value);
};

}

#endif /* SPECTMORPH_BLOCK_UTILS_HH */
