// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_RANDOM_HH
#define SPECTMORPH_RANDOM_HH

#include <stdlib.h>
#include <stdio.h>
#include <rapicorn.hh>

namespace SpectMorph
{

class Random
{
  Rapicorn::Pcg32Rng rand_gen;
public:
  Random();

  void set_seed (uint32_t seed);

  inline double
  random_double_range (double begin, double end)
  {
    const uint32_t  rand_max = 0xffffffff;    // Pcg32Rng output: complete 32-bit values
    const uint32_t  r = random_uint32();
    const double    scale = 1.0 / (double (rand_max) + 1.0);

    return r * scale * (end - begin) + begin;
  }
  inline uint32_t
  random_uint32()
  {
    return rand_gen.random();
  }
  inline void
  random_block (size_t n_values, uint32_t *values)
  {
    while (n_values--)
      *values++ = random_uint32();
  }
};

}
#endif
