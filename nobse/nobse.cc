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

void
sfi_error (const char *msg, ...)
{
  NO_BSE_NO_IMPL (sfi_error);
}

gdouble
bse_db_to_factor (gdouble dB)
{
  double factor = dB / 20; /* Bell */
  return pow (10, factor);
}

gdouble
bse_db_from_factor (gdouble        factor,
                    gdouble        min_dB)
{
  if (factor > 0)
    {
      double dB = log10 (factor); /* Bell */
      dB *= 20;
      return dB;
    }
  else
    return min_dB;
}

int
bse_fpu_okround()
{
  typedef unsigned short int BseFpuState;

  BseFpuState cv;
  __asm__ ("fnstcw %0"
           : "=m" (*&cv));
  return !(cv & 0x0c00);
}

double
bse_window_cos (double x) /* von Hann window */
{
  if (fabs (x) > 1)
    return 0;
  return 0.5 * cos (x * M_PI) + 0.5;
}

double
bse_window_blackman (double x)
{
  if (fabs (x) > 1)
    return 0;
  return 0.42 + 0.5 * cos (M_PI * x) + 0.08 * cos (2.0 * M_PI * x);
}

double
bse_window_hamming (double x) /* sharp (rectangle) cutoffs at boundaries */
{
  if (fabs (x) > 1)
    return 0;

  return 0.54 + 0.46 * cos (M_PI * x);
}

void
gsl_power2_fftar (const uint         n_values,
                  const double      *r_values_in,
                  double            *ri_values_out)
{
  NO_BSE_NO_IMPL (gsl_power2_fftar);
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
