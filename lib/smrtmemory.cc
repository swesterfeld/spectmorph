// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smutils.hh"
#include "smmath.hh"
#include "smrtmemory.hh"

using namespace SpectMorph;

namespace
{

struct PartialData
{
  uint16_t freq;
  uint16_t mag;
};

static bool
pd_cmp (const PartialData& p1, const PartialData& p2)
{
  return p1.freq < p2.freq;
}

}

void
RTAudioBlock::sort_freqs()
{
  // sort partials by frequency
  const size_t N = freqs.size();
  PartialData pvec[N + AVOID_ARRAY_UB];

  for (size_t p = 0; p < N; p++)
    {
      pvec[p].freq = freqs[p];
      pvec[p].mag = mags[p];
    }
  std::sort (pvec, pvec + N, pd_cmp);

  // replace partial data with sorted partial data
  for (size_t p = 0; p < N; p++)
    {
      freqs[p] = pvec[p].freq;
      mags[p] = pvec[p].mag;
    }
}
