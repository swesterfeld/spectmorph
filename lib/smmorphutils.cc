// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphutils.hh"
#include "smmath.hh"

#include <algorithm>

using std::vector;

namespace SpectMorph
{

namespace MorphUtils
{

static bool
fs_cmp (const FreqState& fs1, const FreqState& fs2)
{
  return fs1.freq_f < fs2.freq_f;
}

bool
find_match (float freq, const FreqState *freq_state, size_t freq_state_size, size_t *index)
{
  const float freq_start = freq - 0.5;
  const float freq_end   = freq + 0.5;

  double min_diff = 1e20;
  size_t best_index = 0; // initialized to avoid compiler warning

  FreqState start_freq_state = {freq_start, 0};
  const FreqState *start_ptr = std::lower_bound (freq_state, freq_state + freq_state_size, start_freq_state, fs_cmp);
  size_t i = start_ptr - freq_state;

  while (i < freq_state_size && freq_state[i].freq_f < freq_end)
    {
      if (!freq_state[i].used)
        {
          double diff = fabs (freq - freq_state[i].freq_f);
          if (diff < min_diff)
            {
              best_index = i;
              min_diff = diff;
            }
        }
      i++;
    }
  if (min_diff < 0.5)
    {
      *index = best_index;
      return true;
    }
  return false;
}

void
init_freq_state (const vector<uint16_t>& fint, FreqState *freq_state)
{
  for (size_t i = 0; i < fint.size(); i++)
    {
      freq_state[i].freq_f = sm_ifreq2freq (fint[i]);
      freq_state[i].used   = 0;
    }
}

}

}
