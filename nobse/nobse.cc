// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "nobse.hh"

#define NO_BSE_NO_IMPL(func) { g_printerr ("[libnobse] not implemented: %s\n", #func); g_assert_not_reached(); }

using std::string;

namespace Birnet
{

string
string_printf (const char *format, ...)
{
  NO_BSE_NO_IMPL (Birnet::string_printf);
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
  NO_BSE_NO_IMPL (bse_db_to_factor);
}

gdouble
bse_db_from_factor (gdouble        factor,
                    gdouble        min_dB)
{
  NO_BSE_NO_IMPL (bse_db_from_factor);
}

int
bse_fpu_okround()
{
  NO_BSE_NO_IMPL (bse_fpu_okround);
}

double
bse_window_cos (double x)
{
  NO_BSE_NO_IMPL (bse_window_cos);
}

double
bse_window_blackman (double x)
{
  NO_BSE_NO_IMPL (bse_window_blackman);
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
  NO_BSE_NO_IMPL (Block::add);
}

void
Block::mul (guint           n_values,
            float          *ovalues,
            const float    *ivalues)
{
  NO_BSE_NO_IMPL (Block::mul);
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
  NO_BSE_NO_IMPL (bse_init_inprocess);
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
