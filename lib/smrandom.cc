// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smrandom.hh"

#include <string.h>

#include <glib.h>

using SpectMorph::Random;

Random::Random()
{
  set_seed (g_random_int());
}

void
Random::set_seed (uint32_t seed)
{
  rand_gen.seed (seed);
}
