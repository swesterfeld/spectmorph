// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_UTILS_HH
#define SPECTMORPH_MORPH_UTILS_HH

#include <algorithm>

namespace SpectMorph
{

namespace MorphUtils
{

struct FreqState
{
  float freq_f;
  int   used;
};

bool find_match (float freq, const FreqState *freq_state, size_t freq_state_size, size_t *index);
void init_freq_state (const std::vector<uint16_t>& fint, FreqState *freq_state);

}

}

#endif
