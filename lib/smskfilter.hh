// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

#pragma once

#include "smpandaresampler.hh"

#include <algorithm>

namespace SpectMorph {

using PandaResampler::Resampler2;

class SKFilter
{
public:
  enum Mode {
    LP1, LP2, LP3, LP4, LP6, LP8,
    BP2, BP4, BP6, BP8,
    HP1, HP2, HP3, HP4, HP6, HP8
  };
private:
  static constexpr size_t LAST_MODE = HP8;
  Mode mode_ = Mode::LP2;
  float freq_ = 440;
  float reso_ = 0;
  float drive_ = 0;
  float global_volume_ = 1;
  bool test_linear_ = false;
  int over_ = 1;
  float freq_warp_factor_ = 0;
  float frequency_range_min_ = 0;
  float frequency_range_max_ = 0;
  float clamp_freq_min_ = 0;
  float clamp_freq_max_ = 0;
  float rate_ = 0;

  static constexpr int MAX_STAGES = 4;
  static constexpr uint MAX_BLOCK_SIZE = 1024;

  struct Channel
  {
    std::unique_ptr<Resampler2> res_up;
    std::unique_ptr<Resampler2> res_down;

    std::array<float, MAX_STAGES> s1;
    std::array<float, MAX_STAGES> s2;
  };
  struct FParams
  {
    std::array<float, MAX_STAGES> k;
    float pre_scale = 1;
    float post_scale = 1;
  };
  static constexpr int
  mode2stages (Mode mode)
  {
    switch (mode)
    {
      case LP3:
      case LP4:
      case BP4:
      case HP3:
      case HP4: return 2;
      case LP6:
      case BP6:
      case HP6: return 3;
      case LP8:
      case BP8:
      case HP8: return 4;
      default:  return 1;
    }
  }

  std::array<Channel, 2> channels_;
  FParams                fparams_;
  bool                   fparams_valid_ = false;

  class RTable {
    std::vector<float> res2_k;
    std::vector<float> res3_k;
    std::vector<float> res4_k;
    static constexpr int TSIZE = 16;
    RTable()
    {
      for (int order = 4; order <= 8; order += 2)
        {
          for (int t = 0; t <= TSIZE + 1; t++)
            {
              double res = std::clamp (double (t) / TSIZE, 0.0, 1.0);

              // R must be in interval [0:1]
              const double R = 1 - res;
              const double r_alpha = std::acos (R) / (order / 2);

              std::vector<double> Rn;
              for (int i = 0; i < order / 2; i++)
                {
                  /* butterworth roots in s, left semi plane */
                  const double bw_s_alpha = M_PI * (4 * i + order + 2) / (2 * order);
                  /* add resonance */
                  Rn.push_back (-cos (bw_s_alpha + r_alpha));
                }

              std::sort (Rn.begin(), Rn.end(), std::greater<double>());

              for (auto xr : Rn)
                {
                  if (order == 4)
                    res2_k.push_back ((1 - xr) * 2);
                  if (order == 6)
                    res3_k.push_back ((1 - xr) * 2);
                  if (order == 8)
                    res4_k.push_back ((1 - xr) * 2);
                }
            }
        }
    }
  public:
    static const RTable&
    the()
    {
      static RTable rtable;
      return rtable;
    }
    void
    interpolate_resonance (float res, int stages, float *k, const std::vector<float>& res_k) const
    {
      auto lerp = [] (float a, float b, float frac) {
        return a + frac * (b - a);
      };

      float fidx = std::clamp (res, 0.f, 1.f) * TSIZE;
      int idx = fidx;
      float frac = fidx - idx;

      for (int s = 0; s < stages; s++)
        {
          k[s] = lerp (res_k[idx * stages + s], res_k[idx * stages + stages + s], frac);
        }
    }
    void
    lookup_resonance (float res, int stages, float *k) const
    {
      if (stages == 2)
        interpolate_resonance (res, stages, k, res2_k);

      if (stages == 3)
        interpolate_resonance (res, stages, k, res3_k);

      if (stages == 4)
        interpolate_resonance (res, stages, k, res4_k);

      if (res > 1)
        k[stages - 1] = res * 2;
    }
  };
  const RTable& rtable_;
public:
  SKFilter (int over) :
    over_ (over),
    rtable_ (RTable::the())
  {
    for (auto& channel : channels_)
      {
        channel.res_up   = std::make_unique<Resampler2> (Resampler2::UP, over_, Resampler2::PREC_72DB);
        channel.res_down = std::make_unique<Resampler2> (Resampler2::DOWN, over_, Resampler2::PREC_72DB);
      }
    set_rate (48000);
    set_frequency_range (10, 24000);
    reset();
  }
private:
  void
  setup_reso_drive (FParams& fparams, float reso, float drive)
  {
    if (test_linear_) // test filter as linear filter; don't do any resonance correction
      {
        const float scale = 1e-5;
        fparams.pre_scale = scale;
        fparams.post_scale = 1 / scale;
        setup_k (fparams, reso);

        return;
      }
    const float db_x2_factor = 0.166096404744368; // 1/(20*log(2)/log(10))
    const float sqrt2 = M_SQRT2;

    // scale signal down (without normalization on output) for negative drive
    float negative_drive_vol = 1;
    if (drive < 0)
      {
        negative_drive_vol = exp2f (drive * db_x2_factor);
        drive = 0;
      }
    // drive resonance boost
    if (drive > 0)
      reso += drive * 0.015f;

    float vol = exp2f ((drive + -18 * reso) * db_x2_factor);

    if (reso < 0.9)
      {
        reso = 1 - (1-reso)*(1-reso)*(1-sqrt2/4);
      }
    else
      {
        reso = 1 - (1-0.9f)*(1-0.9f)*(1-sqrt2/4) + (reso-0.9f)*0.1f;
      }

    vol *= global_volume_;
    fparams.pre_scale = negative_drive_vol * vol;
    fparams.post_scale = std::max (1 / vol, 1.0f);
    setup_k (fparams, reso);
  }
  void
  setup_k (FParams& fparams, float res)
  {
    if (mode2stages (mode_) == 1)
      {
        // just one stage
        fparams.k[0] = res * 2;
      }
    else
      {
        rtable_.lookup_resonance (res, mode2stages (mode_), fparams.k.data());
      }
  }
  void
  zero_fill_state()
  {
    for (auto& channel : channels_)
      {
        std::fill (channel.s1.begin(), channel.s1.end(), 0.0);
        std::fill (channel.s2.begin(), channel.s2.end(), 0.0);
      }
  }
public:
  void
  set_mode (Mode m)
  {
    if (mode_ != m)
      {
        mode_ = m;

        zero_fill_state();
        fparams_valid_ = false;
      }
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
  reset ()
  {
    for (auto& channel : channels_)
      {
        channel.res_up->reset();
        channel.res_down->reset();
      }
    zero_fill_state();
    fparams_valid_ = false;
  }
  void
  set_rate (float rate)
  {
    freq_warp_factor_ = 4 / (rate * over_);
    rate_ = rate;

    update_frequency_range();
  }
  void
  set_frequency_range (float min_freq, float max_freq)
  {
    frequency_range_min_ = min_freq;
    frequency_range_max_ = max_freq;

    update_frequency_range();
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
  float
  cutoff_warp (float freq)
  {
    float x = freq * freq_warp_factor_;

    /* approximate tan (pi*x/4) for cutoff warping */
    const float c1 = -3.16783027;
    const float c2 =  0.134516124;
    const float c3 = -4.033321984;

    float x2 = x * x;

    return x * (c1 + c2 * x2) / (c3 + x2);
  }
  static float
  tanh_approx (float x)
  {
    // https://www.musicdsp.org/en/latest/Other/238-rational-tanh-approximation.html
    x = std::clamp (x, -3.0f, 3.0f);

    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
  }
  template<Mode MODE, bool STEREO>
  [[gnu::flatten]]
  void
  process (float *left, float *right, float freq, uint n_samples)
  {
    float g = cutoff_warp (std::clamp (freq, clamp_freq_min_, clamp_freq_max_));
    float G = g / (1 + g);

    for (int stage = 0; stage < mode2stages (MODE); stage++)
      {
        const float k = fparams_.k[stage];

        float xnorm = 1.f / (1 - k * G + k * G * G);
        float s1feedback = -xnorm * k * (G - 1) / (1 + g);
        float s2feedback = -xnorm * k / (1 + g);

        auto lowpass = [G] (float in, float& state)
          {
            float v = G * (in - state);
            float y = v + state;
            state = y + v;
            return y;
          };

        auto mode_out = [] (float y0, float y1, float y2, bool last_stage) -> float
          {
            float y1hp = y0 - y1;
            float y2hp = y1 - y2;

            switch (MODE)
              {
                case LP2:
                case LP4:
                case LP6:
                case LP8: return y2;
                case BP2:
                case BP4:
                case BP6:
                case BP8: return y2hp;
                case HP2:
                case HP4:
                case HP6:
                case HP8: return (y1hp - y2hp);
                case LP1:
                case LP3: return last_stage ? y1 : y2;
                case HP1:
                case HP3: return last_stage ? y1hp : (y1hp - y2hp);
             }
          };

        float s1l, s1r, s2l, s2r;

        s1l = channels_[0].s1[stage];
        s2l = channels_[0].s2[stage];

        if (STEREO)
          {
            s1r = channels_[1].s1[stage];
            s2r = channels_[1].s2[stage];
          }

        auto tick = [&] (uint i, bool last_stage, float pre_scale, float post_scale)
          {
            float xl, xr, y0l, y0r, y1l, y1r, y2l, y2r;

            /*
             * interleaving processing of both channels performs better than
             * processing left and right channel seperately (measured on Ryzen7)
             */

                        { xl = left[i] * pre_scale; }
            if (STEREO) { xr = right[i] * pre_scale; }

                        { y0l = xl * xnorm + s1l * s1feedback + s2l * s2feedback; }
            if (STEREO) { y0r = xr * xnorm + s1r * s1feedback + s2r * s2feedback; }

            if (last_stage)
              {
                            { y0l = tanh_approx (y0l); }
                if (STEREO) { y0r = tanh_approx (y0r); }
              }
                        { y1l = lowpass (y0l, s1l); }
            if (STEREO) { y1r = lowpass (y0r, s1r); }

                        { y2l = lowpass (y1l, s2l); }
            if (STEREO) { y2r = lowpass (y1r, s2r); }

                        { left[i]  = mode_out (y0l, y1l, y2l, last_stage) * post_scale; }
            if (STEREO) { right[i] = mode_out (y0r, y1r, y2r, last_stage) * post_scale; }
          };

        const bool last_stage = mode2stages (MODE) == (stage + 1);

        if (last_stage)
          {
            for (uint i = 0; i < n_samples; i++)
              tick (i, true, fparams_.pre_scale, fparams_.post_scale);
          }
        else
          {
            for (uint i = 0; i < n_samples; i++)
              tick (i, false, 1, 1);
          }

        channels_[0].s1[stage] = s1l;
        channels_[0].s2[stage] = s2l;

        if (STEREO)
          {
            channels_[1].s1[stage] = s1r;
            channels_[1].s2[stage] = s2r;
          }
      }
  }
  template<Mode MODE>
  void
  process_block_mode (uint n_samples, float *left, float *right, const float *freq_in, const float *reso_in, const float *drive_in)
  {
    float over_samples_left[n_samples * over_];
    float over_samples_right[n_samples * over_];

    /* we only support stereo (left != 0, right != 0) and mono (left != 0, right == 0) */
    bool stereo = left && right;

    channels_[0].res_up->process_block (left, n_samples, over_samples_left);
    if (stereo)
      channels_[1].res_up->process_block (right, n_samples, over_samples_right);

    if (!fparams_valid_)
      {
        setup_reso_drive (fparams_, reso_in ? reso_in[0] : reso_, drive_in ? drive_in[0] : drive_);
        fparams_valid_ = true;
      }

    if (reso_in || drive_in)
      {
        /* for reso or drive modulation, we split the input it into small blocks
         * and interpolate the pre_scale / post_scale / k parameters
         */
        float *left_blk = over_samples_left;
        float *right_blk = over_samples_right;

        uint n_remaining_samples = n_samples;
        while (n_remaining_samples)
          {
            const uint todo = std::min<uint> (n_remaining_samples, 64);

            FParams fparams_end;
            setup_reso_drive (fparams_end, reso_in ? reso_in[todo - 1] : reso_, drive_in ? drive_in[todo - 1] : drive_);

            constexpr static int STAGES = mode2stages (MODE);
            float todo_inv = 1.f / todo;
            float delta_pre_scale = (fparams_end.pre_scale - fparams_.pre_scale) * todo_inv;
            float delta_post_scale = (fparams_end.post_scale - fparams_.post_scale) * todo_inv;
            float delta_k[STAGES];
            for (int stage = 0; stage < STAGES; stage++)
              delta_k[stage] = (fparams_end.k[stage] - fparams_.k[stage]) * todo_inv;

            uint j = 0;
            for (uint i = 0; i < todo * over_; i += over_)
              {
                fparams_.pre_scale += delta_pre_scale;
                fparams_.post_scale += delta_post_scale;

                for (int stage = 0; stage < STAGES; stage++)
                  fparams_.k[stage] += delta_k[stage];

                float freq = freq_in ? freq_in[j++] : freq_;

                if (stereo)
                  {
                    process<MODE, true>  (left_blk + i, right_blk + i, freq, over_);
                  }
                else
                  {
                    process<MODE, false> (left_blk + i, nullptr, freq, over_);
                  }
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
        uint j = 0;
        for (uint i = 0; i < n_samples * over_; i += over_)
          {
            float freq = freq_in[j++];

            if (stereo)
              {
                process<MODE, true>  (over_samples_left + i, over_samples_right + i, freq, over_);
              }
            else
              {
                process<MODE, false> (over_samples_left + i, nullptr, freq, over_);
              }
          }
      }
    else
      {
        if (stereo)
          {
            process<MODE, true>  (over_samples_left, over_samples_right, freq_, n_samples * over_);
          }
        else
          {
            process<MODE, false> (over_samples_left, nullptr, freq_, n_samples * over_);
          }
      }

    channels_[0].res_down->process_block (over_samples_left, n_samples * over_, left);
    if (stereo)
      channels_[1].res_down->process_block (over_samples_right, n_samples * over_, right);
  }

  using ProcessBlockFunc = decltype (&SKFilter::process_block_mode<LP2>);

  template<size_t... INDICES>
  static constexpr std::array<ProcessBlockFunc, LAST_MODE + 1>
  make_jump_table (std::integer_sequence<size_t, INDICES...>)
  {
    auto mk_func = [] (auto I) { return &SKFilter::process_block_mode<Mode (I.value)>; };

    return { mk_func (std::integral_constant<int, INDICES>{})... };
  }
public:
  void
  process_block (uint n_samples, float *left, float *right = nullptr, const float *freq_in = nullptr, const float *reso_in = nullptr, const float *drive_in = nullptr)
  {
    static constexpr auto jump_table { make_jump_table (std::make_index_sequence<LAST_MODE + 1>()) };

    while (n_samples)
      {
        const uint todo = std::min (n_samples, MAX_BLOCK_SIZE);

        (this->*jump_table[mode_]) (todo, left, right, freq_in, reso_in, drive_in);

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
