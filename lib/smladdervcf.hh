// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

#pragma once

#include "smpandaresampler.hh"

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace SpectMorph {

using PandaResampler::Resampler2;

class LadderVCF
{
public:
  enum Mode {
    LP1, LP2, LP3, LP4
  };
private:
  struct Channel {
    float x1, x2, x3, x4;
    float y1, y2, y3, y4;

    std::unique_ptr<Resampler2> res_up;
    std::unique_ptr<Resampler2> res_down;
  };
  std::array<Channel, 2> channels_;
  Mode mode_;
  float rate_ = 0;
  float freq_scale_factor_ = 0;
  float frequency_range_min_ = 0;
  float frequency_range_max_ = 0;
  float clamp_freq_min_ = 0;
  float clamp_freq_max_ = 0;
  float freq_ = 440;
  float reso_ = 0;
  float drive_ = 0;
  float global_volume_ = 1;
  uint over_ = 0;
  bool test_linear_ = false;

  static constexpr uint MAX_BLOCK_SIZE = 1024;

  struct FParams
  {
    float reso = 0;
    float pre_scale = 1;
    float post_scale = 1;
  };
  FParams fparams_;
  bool    fparams_valid_ = false;
public:
  LadderVCF (int over) :
    over_ (over)
  {
    for (auto& channel : channels_)
      {
        channel.res_up   = std::make_unique<Resampler2> (Resampler2::UP, over_, Resampler2::PREC_72DB);
        channel.res_down = std::make_unique<Resampler2> (Resampler2::DOWN, over_, Resampler2::PREC_72DB);
      }
    set_mode (Mode::LP4);
    set_rate (48000);
    set_frequency_range (10, 24000);
    reset();
  }
  void
  set_mode (Mode new_mode)
  {
    mode_ = new_mode;
  }
  void
  set_freq (float freq)
  {
    freq_ = freq;
  }
  void
  set_reso (float reso)
  {
    reso_ = reso;
    fparams_valid_ = false;
  }
  void
  set_drive (float drive)
  {
    drive_ = drive;
    fparams_valid_ = false;
  }
  void
  set_global_volume (float global_volume)
  {
    /* every samples that is processed by the filter is
     *  - multiplied with global_volume before processing
     *  - divided by global_volume after processing
     * which has an effect on the non-linear part of the filter (drive)
     */
    global_volume_ = global_volume;
    fparams_valid_ = false;
  }
  void
  set_test_linear (bool test_linear)
  {
    test_linear_ = test_linear;
    fparams_valid_ = false;
  }
  void
  set_rate (float r)
  {
    rate_ = r;
    freq_scale_factor_ = 2 * M_PI / (rate_ * over_);

    update_frequency_range();
  }
  void
  set_frequency_range (float min_freq, float max_freq)
  {
    frequency_range_min_ = min_freq;
    frequency_range_max_ = max_freq;

    update_frequency_range();
  }
  void
  reset()
  {
    for (auto& c : channels_)
      {
        c.x1 = c.x2 = c.x3 = c.x4 = 0;
        c.y1 = c.y2 = c.y3 = c.y4 = 0;

        c.res_up->reset();
        c.res_down->reset();
      }
    fparams_valid_ = false;
  }
  double
  delay()
  {
    return channels_[0].res_up->delay() / over_ + channels_[0].res_down->delay();
  }
private:
  void
  update_frequency_range()
  {
    /* we want to clamp to the user defined range (set_frequency_range())
     * but also enforce that the filter is well below nyquist frequency
     */
    clamp_freq_min_ = frequency_range_min_;
    clamp_freq_max_ = std::min (frequency_range_max_, rate_ * over_ * 0.49f);
  }
  void
  setup_reso_drive (FParams& fparams, float reso, float drive)
  {
    reso = std::clamp (reso, 0.001f, 1.f);

    if (test_linear_) // test filter as linear filter; don't do any resonance correction
      {
        const float scale = 1e-5;
        fparams.pre_scale = scale;
        fparams.post_scale = 1 / scale;
        fparams.reso = reso * 4;

        return;
      }
    const float db_x2_factor = 0.166096404744368; // 1/(20*log(2)/log(10))

    // scale signal down (without normalization on output) for negative drive
    float negative_drive_vol = 1;
    if (drive < 0)
      {
        negative_drive_vol = exp2f (drive * db_x2_factor);
        drive = 0;
      }
    // drive resonance boost
    if (drive > 0)
      reso += drive * sqrt (reso) * reso * 0.03f;

    float vol = exp2f ((drive + -12 * sqrt (reso)) * db_x2_factor);
    vol *= global_volume_;
    fparams.pre_scale = negative_drive_vol * vol;
    fparams.post_scale = std::max (1 / vol, 1.0f);
    fparams.reso = sqrt (reso) * 4;
  }
  static float
  tanh_approx (float x)
  {
    // https://www.musicdsp.org/en/latest/Other/238-rational-tanh-approximation.html
    x = std::clamp (x, -3.0f, 3.0f);

    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
  }
  /*
   * This ladder filter implementation is mainly based on
   *
   * Välimäki, Vesa & Huovilainen, Antti. (2006).
   * Oscillator and Filter Algorithms for Virtual Analog Synthesis.
   * Computer Music Journal. 30. 19-31. 10.1162/comj.2006.30.2.19.
   */
  template<Mode MODE, bool STEREO> inline void
  run (float *left, float *right, float freq, uint n_samples)
  {
    const float fc = std::clamp (freq, clamp_freq_min_, clamp_freq_max_) * freq_scale_factor_;
    const float g = 0.9892f * fc - 0.4342f * fc * fc + 0.1381f * fc * fc * fc - 0.0202f * fc * fc * fc * fc;
    const float b0 = g * (1 / 1.3f);
    const float b1 = g * (0.3f / 1.3f);
    const float a1 = g - 1;

    float res = fparams_.reso;
    res *= 1.0029f + 0.0526f * fc - 0.0926f * fc * fc + 0.0218f * fc * fc * fc;

    for (uint os = 0; os < n_samples; os++)
      {
        for (uint i = 0; i < (STEREO ? 2 : 1); i++)
          {
            float &value = i == 0 ? left[os] : right[os];

            Channel& c = channels_[i];
            const float x = value * fparams_.pre_scale;
            const float g_comp = 0.5f; // passband gain correction
            const float x0 = tanh_approx (x - (c.y4 - g_comp * x) * res);

            c.y1 = b0 * x0 + b1 * c.x1 - a1 * c.y1;
            c.x1 = x0;

            c.y2 = b0 * c.y1 + b1 * c.x2 - a1 * c.y2;
            c.x2 = c.y1;

            c.y3 = b0 * c.y2 + b1 * c.x3 - a1 * c.y3;
            c.x3 = c.y2;

            c.y4 = b0 * c.y3 + b1 * c.x4 - a1 * c.y4;
            c.x4 = c.y3;

            switch (MODE)
              {
                case LP1:
                  value = c.y1 * fparams_.post_scale;
                  break;
                case LP2:
                  value = c.y2 * fparams_.post_scale;
                  break;
                case LP3:
                  value = c.y3 * fparams_.post_scale;
                  break;
                case LP4:
                  value = c.y4 * fparams_.post_scale;
                  break;
                default:
                  assert (false);
              }
          }
      }
  }
  template<Mode MODE, bool STEREO> inline void
  do_process_block (uint          n_samples,
                    float        *left,
                    float        *right,
                    const float  *freq_in,
                    const float  *reso_in,
                    const float  *drive_in)
  {
    float over_samples_left[over_ * n_samples];
    float over_samples_right[over_ * n_samples];

    channels_[0].res_up->process_block (left, n_samples, over_samples_left);
    if (STEREO)
      channels_[1].res_up->process_block (right, n_samples, over_samples_right);

    if (!fparams_valid_)
      {
        setup_reso_drive (fparams_, reso_in ? reso_in[0] : reso_, drive_in ? drive_in[0] : drive_);
        fparams_valid_ = true;
      }

    if (reso_in || drive_in)
      {
        /* for reso or drive modulation, we split the input it into small blocks
         * and interpolate the pre_scale / post_scale / reso parameters
         */
        float *left_blk = over_samples_left;
        float *right_blk = over_samples_right;

        uint n_remaining_samples = n_samples;
        while (n_remaining_samples)
          {
            const uint todo = std::min<uint> (n_remaining_samples, 64);

            FParams fparams_end;
            setup_reso_drive (fparams_end, reso_in ? reso_in[todo - 1] : reso_, drive_in ? drive_in[todo - 1] : drive_);

            float todo_inv = 1.f / todo;
            float delta_pre_scale = (fparams_end.pre_scale - fparams_.pre_scale) * todo_inv;
            float delta_post_scale = (fparams_end.post_scale - fparams_.post_scale) * todo_inv;
            float delta_reso = (fparams_end.reso - fparams_.reso) * todo_inv;

            uint j = 0;
            for (uint i = 0; i < todo * over_; i += over_)
              {
                fparams_.pre_scale += delta_pre_scale;
                fparams_.post_scale += delta_post_scale;
                fparams_.reso += delta_reso;

                float freq = freq_in ? freq_in[j++] : freq_;

                run<MODE, STEREO> (left_blk + i, right_blk + i, freq, over_);
              }

            n_remaining_samples -= todo;
            left_blk += todo * over_;
            right_blk += todo * over_;

            if (freq_in)
              freq_in += todo;
            if (reso_in)
              reso_in += todo;
            if (drive_in)
              drive_in += todo;
          }
      }
    else if (freq_in)
      {
        uint over_pos = 0;

        for (uint i = 0; i < n_samples; i++)
          {
            run<MODE, STEREO> (over_samples_left + over_pos, over_samples_right + over_pos, freq_in[i], over_);
            over_pos += over_;
          }
      }
    else
      {
        run<MODE, STEREO> (over_samples_left, over_samples_right, freq_, n_samples * over_);
      }
    channels_[0].res_down->process_block (over_samples_left, over_ * n_samples, left);
    if (STEREO)
      channels_[1].res_down->process_block (over_samples_right, over_ * n_samples, right);
  }
  template<Mode MODE> inline void
  process_block_mode (uint          n_samples,
                      float        *left,
                      float        *right,
                      const float  *freq_in,
                      const float  *reso_in,
                      const float  *drive_in)
  {
    if (right) // stereo?
      do_process_block<MODE, true> (n_samples, left, right, freq_in, reso_in, drive_in);
    else
      do_process_block<MODE, false> (n_samples, left, right, freq_in, reso_in, drive_in);
  }
public:
  void
  process_block (uint         n_samples,
                 float       *left,
                 float       *right = nullptr,
                 const float *freq_in = nullptr,
                 const float *reso_in = nullptr,
                 const float *drive_in = nullptr)
  {
    while (n_samples)
      {
        const uint todo = std::min (n_samples, MAX_BLOCK_SIZE);

        switch (mode_)
          {
            case LP4: process_block_mode<LP4> (todo, left, right, freq_in, reso_in, drive_in);
                      break;
            case LP3: process_block_mode<LP3> (todo, left, right, freq_in, reso_in, drive_in);
                      break;
            case LP2: process_block_mode<LP2> (todo, left, right, freq_in, reso_in, drive_in);
                      break;
            case LP1: process_block_mode<LP1> (todo, left, right, freq_in, reso_in, drive_in);
                      break;
          }

        if (left)
          left += todo;
        if (right)
          right += todo;
        if (freq_in)
          freq_in += todo;
        if (reso_in)
          reso_in += todo;
        if (drive_in)
          drive_in += todo;

        n_samples -= todo;
      }
  }
};

} // SpectMorph
