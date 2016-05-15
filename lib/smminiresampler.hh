// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MINI_RESAMPLER_HH
#define SPECTMORPH_MINI_RESAMPLER_HH

#include <bse/gsldatautils.hh>

#include <vector>

namespace SpectMorph
{

class MiniResampler
{
  GslDataHandle    *m_dhandle;
  GslDataPeekBuffer m_peek_buffer;
  double            m_speedup_factor;
public:
  MiniResampler (GslDataHandle *dhandle, double speedup_factor);

  int read (uint64 pos, size_t block_size, float *out);
  uint64 length();
};

};

#endif
