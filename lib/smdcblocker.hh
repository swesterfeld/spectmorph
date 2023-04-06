// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

#pragma once

#include <cmath>
#include <cassert>

/* 1st/2nd order IIR high pass filter to block DC offset; this is a biquad
 * filter, however, the b0, b1, b2 are omitted in the design and simplified
 * in the evaluation of the filter because they are close to integers
 */
class DCBlocker
{
  float a1_ = 0;
  float a2_ = 0;

  struct BiquadState
  {
    float x1 = 0;
    float x2 = 0;
    float y1 = 0;
    float y2 = 0;

    void
    reset()
    {
      x1 = 0;
      x2 = 0;
      y1 = 0;
      y2 = 0;
    }
  } state_left_, state_right_;

  float
  apply_biquad1p (float in, BiquadState& state)
  {
    float out = in - state.x1 - a1_ * state.y1;
    state.x1 = in;
    state.y1 = out;
    return out;
  }

  float
  apply_biquad2p (float in, BiquadState& state)
  {
    float out = in - 2 * state.x1 + state.x2 - a1_ * state.y1 - a2_ * state.y2;
    state.x2 = state.x1;
    state.x1 = in;
    state.y2 = state.y1;
    state.y1 = out;
    return out;
  }

  template<int CHANNELS> void
  process_impl (uint n_samples, float *left, float *right)
  {
    assert (order_ != 0);

    BiquadState sl = state_left_, sr = state_right_;

    if (order_ == 1)
      {
        for (uint i = 0; i < n_samples; i++)
          {
            left[i] = apply_biquad1p (left[i], sl);
            if constexpr (CHANNELS == 2)
              right[i] = apply_biquad1p (right[i], sr);
          }
      }
    else
      {
        for (uint i = 0; i < n_samples; i++)
          {
            left[i] = apply_biquad2p (left[i], sl);
            if constexpr (CHANNELS == 2)
              right[i] = apply_biquad2p (right[i], sr);
          }
      }

    state_left_ = sl;
    state_right_ = sr;
  }
  int order_ = 0;
public:
  void
  reset (float freq, float rate, int order)
  {
    assert (order == 1 || order == 2);

    const double k = M_PI * freq / rate;

    if (order == 1)
      {
        a1_ = (k - 1) / (k + 1);
        a2_ = 0;
      }
    else
      {
        const double kk = k * k;
        const double div_factor = 1 / (1 + (k + M_SQRT2) * k);

        a1_ = 2 * (kk - 1) * div_factor;
        a2_ = (1 - k * M_SQRT2 + kk) * div_factor;
      }
    state_left_.reset();
    state_right_.reset();
    order_ = order;
  }
  void
  process (uint n_samples, float *left, float *right = nullptr)
  {
    if (right)
      process_impl<2> (n_samples, left, right);
    else
      process_impl<1> (n_samples, left, nullptr);
  }
};
