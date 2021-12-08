// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smminiresampler.hh"
#include <string.h>
#include <glib.h>

#include "config.h"

#if SPECTMORPH_HAVE_BSE
#include <bse/gsldatautils.hh>
#endif

using std::vector;
using std::min;

using namespace SpectMorph;

MiniResampler::MiniResampler (const WavData& wav_data, double speedup_factor)
{
#if !SPECTMORPH_HAVE_BSE
  g_printerr ("SpectMorph::MiniResampler: not supported without libbse\n");
  g_assert_not_reached();
#else
  const vector<float>& samples = wav_data.samples();
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, wav_data.mix_freq(), 440, samples.size(), &samples[0], NULL);

  while (speedup_factor < 6)
    {
      dhandle = bse_data_handle_new_upsample2 (dhandle, 24);
      speedup_factor *= 2;
      Bse::Error error = gsl_data_handle_open (dhandle);
      if (error != 0)
        {
          fprintf (stderr, "foo\n");
          exit (1);
        }
    }

  GslDataPeekBuffer peek_buffer = { 0, };

  uint64 n_values = gsl_data_handle_n_values (dhandle);
  for (size_t pos = 0; ; pos++)
    {
      // linear interpolation
      double dpos = pos * speedup_factor;
      uint64 left_pos = dpos;
      uint64 right_pos = left_pos + 1;
      if (right_pos >= n_values)  // read past eof
        break;
      double fade = dpos - left_pos;     // 0 => left sample 0.5 => mix both 1.0 => right sample
      m_samples.push_back ((1 - fade) * gsl_data_handle_peek_value (dhandle, left_pos, &peek_buffer)
                         + fade * gsl_data_handle_peek_value (dhandle, right_pos, &peek_buffer));
    }
#endif
}

int
MiniResampler::read (size_t pos, size_t block_size, float *out)
{
  const size_t start = min (m_samples.size(), pos);
  const size_t end   = min (m_samples.size(), pos + block_size);

  std::copy (&m_samples[start], &m_samples[end], out);

  return end - start;
}

uint64
MiniResampler::length()
{
  return m_samples.size();
}


