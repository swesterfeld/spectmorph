// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smblockutils.hh"

using namespace SpectMorph;

void
Block::add (guint           n_values,
            float          *ovalues,
            const float    *ivalues)
{
  // auto vectorization for simple cases is quite good these days,
  // so we don't provide an SSE intrinsics implementation

  for (guint i = 0; i < n_values; i++)
    ovalues[i] += ivalues[i];
}

void
Block::mul (guint           n_values,
            float          *ovalues,
            const float    *ivalues)
{
  // auto vectorization for simple cases is quite good these days,
  // so we don't provide an SSE intrinsics implementation

  for (guint i = 0; i < n_values; i++)
    ovalues[i] *= ivalues[i];
}

void
Block::range (guint           n_values,
              const float    *ivalues,
              float&          min_value,
              float&          max_value)
{
  float minv, maxv;
  if (n_values)
    {
      minv = maxv = ivalues[0];

      for (guint i = 1; i < n_values; i++)
        {
          if (G_UNLIKELY (ivalues[i] < minv))
            minv = ivalues[i];
          if (G_UNLIKELY (ivalues[i] > maxv))
            maxv = ivalues[i];
        }
    }
  else
    {
      minv = maxv = 0;
    }
  min_value = minv;
  max_value = maxv;
}
