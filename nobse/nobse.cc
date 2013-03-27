// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "nobse.hh"

#include <math.h>

#define NO_BSE_NO_IMPL(func) { g_printerr ("[libnobse] not implemented: %s\n", #func); g_assert_not_reached(); }

using std::string;

namespace Birnet
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

string
string_vprintf (const char *format,
                va_list     vargs)
{
  char *str = NULL;
  if (vasprintf (&str, format, vargs) >= 0 && str)
    {
      string s = str;
      free (str);
      return s;
    }
  else
    return format;
}

string
string_printf (const char *format, ...)
{
  string str;
  va_list args;
  va_start (args, format);
  str = string_vprintf (format, args);
  va_end (args);
  return str;
}

}

const gchar*
bse_error_blurb (BseErrorType error_value)
{
  NO_BSE_NO_IMPL (bse_error_blurb);
}

BseErrorType
gsl_data_handle_open (GslDataHandle *dhandle)
{
  NO_BSE_NO_IMPL (gsl_data_handle_open);
}

void
gsl_data_handle_close (GslDataHandle *dhandle)
{
  NO_BSE_NO_IMPL (gsl_data_handle_close);
}

int64
gsl_data_handle_length (GslDataHandle *data_handle)
{
  NO_BSE_NO_IMPL (gsl_data_handle_length);
}

int64
gsl_data_handle_read (GslDataHandle	  *data_handle,
		      int64		   value_offset,
		      int64		   n_values,
		      gfloat		  *values)
{
  NO_BSE_NO_IMPL (gsl_data_handle_read);
}

guint
gsl_data_handle_n_channels (GslDataHandle *data_handle)
{
  NO_BSE_NO_IMPL (gsl_data_handle_n_channels);
}

gfloat
gsl_data_handle_mix_freq (GslDataHandle *data_handle)
{
  NO_BSE_NO_IMPL (gsl_data_handle_mix_freq);
}

GslDataHandle*
gsl_data_handle_new_mem (guint		   n_channels,
			 guint             bit_depth,
			 gfloat            mix_freq,
			 gfloat            osc_freq,
			 int64		   n_values,
			 const gfloat	  *values,
			 void              (*free) (gpointer values))
{
  NO_BSE_NO_IMPL (gsl_data_handle_new_mem);
}

GslDataHandle*
gsl_data_handle_new_insert (GslDataHandle	  *src_handle,
			    guint                  pasted_bit_depth,
			    int64		   insertion_offset,
			    int64		   n_paste_values,
			    const gfloat	  *paste_values,
			    void                   (*free) (gpointer values))
{
  NO_BSE_NO_IMPL (gsl_data_handle_new_insert);
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
bse_window_cos (double x)
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
  NO_BSE_NO_IMPL (Block::range);
}

} // namespace Bse

double
BSE_SIGNAL_TO_FREQ (double sig)
{
  NO_BSE_NO_IMPL (BSE_SIGNAL_TO_FREQ);
}

void
bse_init_inprocess (gint           *argc,
                    gchar        ***argv,
                    const char     *app_name,
                    SfiInitValue    values[])
{
  // nothing
}

BseWaveFileInfo*
bse_wave_file_info_load (const gchar     *file_name,
                         BseErrorType    *error)
{
  NO_BSE_NO_IMPL (bse_wave_file_info_load);
}

BseWaveDsc*
bse_wave_dsc_load (BseWaveFileInfo *wave_file_info,
                   guint            nth_wave,
                   gboolean         accept_empty,
                   BseErrorType    *error)
{
  NO_BSE_NO_IMPL (bse_wave_dsc_load);
}

GslDataHandle*
bse_wave_handle_create (BseWaveDsc      *wave_dsc,
                        guint            nth_chunk,
                        BseErrorType    *error)
{
  NO_BSE_NO_IMPL (bse_wave_handle_create);
}
