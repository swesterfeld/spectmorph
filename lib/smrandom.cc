// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smrandom.hh"

#include <string.h>

#include <glib.h>

using SpectMorph::Random;

Random::Random()
{
  memset (&buf, 0, sizeof (buf));
  for (size_t i = 0; i < sizeof (state); i++)
    state[i] = g_random_int();

  initstate_r (g_random_int(), state, sizeof (state), &buf);
}

void
Random::set_seed (int seed)
{
  srandom_r (seed, &buf);
}
