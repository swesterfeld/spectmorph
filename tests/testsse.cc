// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"

#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

#if defined (__SSE__) || defined (SM_ARM_SSE)
bool
mm_expect (__m128 m, float f3, float f2, float f1, float f0)
{
  F4Vector f;
  f.v = m;
  bool ok = (f0 == f.f[0]) && (f1 == f.f[1]) && (f2 == f.f[2]) && (f3 == f.f[3]);
  if (!ok)
    {
      printf ("got %f %f %f %f\n", f.f[0], f.f[1], f.f[2], f.f[3]);
      printf ("expect %f %f %f %f\n", f0, f1, f2, f3);
    }
  return ok;
}

bool
mm_expect0 (__m128 m, float f0)
{
  F4Vector f;
  f.v = m;
  bool ok = f0 == f.f[0];
  if (!ok)
    {
      printf ("expect %f\n", f.f[0]);
      printf ("got %f\n", f0);
    }
  return ok;
}
#endif

int
main (int argc, char **argv)
{
#if defined (__SSE__) || defined (SM_ARM_SSE)
  assert (mm_expect (_mm_set_ps (10, 20, 30, 40),
                     10, 20, 30, 40));
  assert (mm_expect (_mm_add_ps (_mm_set_ps (0, 1, 2, 3),
                                 _mm_set_ps (2, 4, 6, 8)),
                     2, 5, 8, 11));
  assert (mm_expect (_mm_mul_ps (_mm_set_ps (0, 1, 2, 3),
                                 _mm_set_ps (2, 4, 6, 8)),
                     0, 4, 12, 24));
  assert (mm_expect (_mm_sub_ps (_mm_set_ps (0, 1, 2, 3),
                                 _mm_set_ps (2, 4, 6, 8)),
                     -2, -3, -4, -5));
  assert (mm_expect0 (_mm_set_ss (42),
                      42));

  float A[4] = { 0, 1, 2, 3 };
  float B[4] = { 4, 5, 6, 7 };
  assert (mm_expect (_mm_shuffle_ps(_mm_set_ps (A[3], A[2], A[1], A[0]),
                                    _mm_set_ps (B[3], B[2], B[1], B[0]), _MM_SHUFFLE (0, 3, 0, 3)),
                     B[0], B[3], A[0], A[3]));
  assert (mm_expect (_mm_shuffle_ps(_mm_set_ps (A[3], A[2], A[1], A[0]),
                                    _mm_set_ps (B[3], B[2], B[1], B[0]), _MM_SHUFFLE (1, 2, 3, 0)),
                     B[1], B[2], A[3], A[0]));

  // test macro argument double evaluation
  int k = 0, j = 0;
  assert (mm_expect (_mm_shuffle_ps (_mm_set_ps (k++, 8, 8, 8),
                                     _mm_set_ps (8, 8, 8, ++j), _MM_SHUFFLE (0, 0, 3, 3)),
                     1, 1, 0, 0));
  assert (k == 1 && j == 1);
  printf ("SSE test ok.\n");
#endif
}
