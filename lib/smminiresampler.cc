// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smminiresampler.hh"
#include <string.h>

using namespace SpectMorph;

MiniResampler::MiniResampler (GslDataHandle *dhandle, double speedup_factor)
{
  m_speedup_factor = speedup_factor;
  m_dhandle = dhandle;
  memset (&m_peek_buffer, 0, sizeof (m_peek_buffer));
  while (m_speedup_factor < 6)
    {
      m_dhandle = bse_data_handle_new_upsample2 (m_dhandle, 24);
      m_speedup_factor *= 2;
      Bse::Error error = gsl_data_handle_open (m_dhandle);
      if (error != 0)
	{
	  fprintf (stderr, "foo\n");
	  exit (1);
	}
    }
}

int
MiniResampler::read (uint64 pos, size_t block_size, float *out)
{
  uint64 n_values = gsl_data_handle_n_values (m_dhandle);
  for (size_t i = 0; i < block_size; i++)
    {
      // linear interpolation
      double dpos = (pos + i) * m_speedup_factor;
      uint64 left_pos = dpos;
      uint64 right_pos = left_pos + 1;
      if (right_pos >= n_values)
	return (i - 1);                  // partial read
      double fade = dpos - left_pos;     // 0 => left sample 0.5 => mix both 1.0 => right sample
      out[i] = (1 - fade) * gsl_data_handle_peek_value (m_dhandle, left_pos, &m_peek_buffer)
	     + fade * gsl_data_handle_peek_value (m_dhandle, right_pos, &m_peek_buffer);
    }
  return block_size;
}

uint64
MiniResampler::length()
{
  return gsl_data_handle_n_values (m_dhandle) / m_speedup_factor;
}


