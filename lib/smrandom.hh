// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_RANDOM_HH
#define SPECTMORPH_RANDOM_HH

#include <stdlib.h>
#include <stdio.h>

#include "smpcg32rng.hh"

namespace SpectMorph
{

class Random
{
  Pcg32Rng rand_gen;
  template<class T>
  inline T
  random_real_range (T begin, T end)
  {
    const uint32_t  rand_max = 0xffffffff;    // Pcg32Rng output: complete 32-bit values
    const uint32_t  r = random_uint32();
    const T         scale = 1 / (T (rand_max) + 1);

    return r * scale * (end - begin) + begin;
  }
public:
  Random();

  void set_seed (uint32_t seed);

  inline double
  random_double_range (double begin, double end)
  {
    return random_real_range<double> (begin, end);
  }
  inline float
  random_float_range (float begin, float end)
  {
    return random_real_range<float> (begin, end);
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
