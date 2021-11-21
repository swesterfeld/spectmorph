// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MATH_ARM_HH
#define SPECTMORPH_MATH_ARM_HH

#if defined(__arm64__) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t __m128;

inline __attribute__((always_inline)) __m128 _mm_set_ps(float e3, float e2, float e1, float e0)
{
  __m128 r;
  alignas(16) float data[4] = {e0, e1, e2, e3};
  r = vld1q_f32(data);
  return r;
}

define _MM_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

inline __attribute__((always_inline)) __m128 _mm_mul_ps(__m128 a, __m128 b)
{
  return vmulq_f32(a, b);
}

inline __attribute__((always_inline)) __m128 _mm_add_ps(__m128 a, __m128 b)
{
  return vaddq_f32(a, b);
}

inline __attribute__((always_inline)) __m128 _mm_sub_ps(__m128 a, __m128 b)
{
  return vsubq_f32(a, b);
}

inline __attribute__((always_inline)) __m128 _mm_set_ss(float a)
{
  return vsetq_lane_f32(a, vdupq_n_f32(0.f), 0);
}

inline __attribute__((always_inline)) __m128 _mm_xor_ps(__m128 a, __m128 b)
{
  return veorq_s32(a, b);
}

#define _mm_shuffle_ps(a, b, imm8)                                                                             \
  __extension__({                                                                                              \
    float32x4_t ret;                                                                                           \
    ret = vmovq_n_f32(vgetq_lane_f32(a, (imm8) & (0x3)));                                                      \
    ret = vsetq_lane_f32(vgetq_lane_f32(a, ((imm8) >> 2) & 0x3), ret, 1);                                      \
    ret = vsetq_lane_f32(vgetq_lane_f32(b, ((imm8) >> 4) & 0x3), ret, 2);                                      \
    ret = vsetq_lane_f32(vgetq_lane_f32(b, ((imm8) >> 6) & 0x3), ret, 3);                                      \
  })
#endif

#endif
