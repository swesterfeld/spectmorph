// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  /* multiply seed with different primes in order to set the generator state to
   * a pseudo-random position derived from seed
   */
  const uint64_t prime1 = 3126986573;
  const uint64_t prime2 = 4151919467;

  rand_gen.seed (prime1 * seed, prime2 * seed);
}
