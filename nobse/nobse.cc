// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "nobse.hh"

#include <math.h>

#define NO_BSE_NO_IMPL(func) { g_printerr ("[libnobse] not implemented: %s\n", #func); g_assert_not_reached(); }

using std::string;
using std::vector;

namespace Rapicorn
{

/* --- memory utils --- */
void*
malloc_aligned (gsize     total_size,
                gsize     alignment,
                guint8  **free_pointer)
{
  const bool  alignment_power_of_2 = (alignment & (alignment - 1)) == 0;
  const gsize cache_line_size = 64; // ensure that no false sharing will occur (at begin and end of data)
  if (alignment_power_of_2)
    {
      // for power of 2 alignment, we guarantee also cache line alignment
      alignment = std::max (alignment, cache_line_size);
      uint8 *aligned_mem = (uint8 *) g_malloc (total_size + (alignment - 1) + (cache_line_size - 1));
      *free_pointer = aligned_mem;
      if ((ptrdiff_t) aligned_mem % alignment)
        aligned_mem += alignment - (ptrdiff_t) aligned_mem % alignment;
      return aligned_mem;
    }
  else
    {
      uint8 *aligned_mem = (uint8 *) g_malloc (total_size + (alignment - 1) + (cache_line_size - 1) * 2);
      *free_pointer = aligned_mem;
      if ((ptrdiff_t) aligned_mem % cache_line_size)
        aligned_mem += cache_line_size - (ptrdiff_t) aligned_mem % cache_line_size;
      if ((ptrdiff_t) aligned_mem % alignment)
        aligned_mem += alignment - (ptrdiff_t) aligned_mem % alignment;
      return aligned_mem;
    }
}

}

namespace Bse
{

void
Block::add (guint           n_values,
            float          *ovalues,
            const float    *ivalues)
{
  for (guint i = 0; i < n_values; i++)
    ovalues[i] += ivalues[i];
}

void
Block::mul (guint           n_values,
            float          *ovalues,
            const float    *ivalues)
{
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

} // namespace Bse

double
BSE_SIGNAL_TO_FREQ (double sig)
{
  NO_BSE_NO_IMPL (BSE_SIGNAL_TO_FREQ);
}

void
bse_init_inprocess (gint           *argc,
                    gchar         **argv,
                    const char     *app_name,
                    const vector<string>& args)
{
  // nothing
}
