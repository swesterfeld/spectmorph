// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

#pragma once

namespace SpectMorph
{

class FlexADSR
{
public:
  enum class Shape { FLEXIBLE, EXPONENTIAL, LINEAR };
private:
  float attack_ = 0;
  float attack_slope_ = 0;
  float decay_ = 0;
  float decay_slope_ = 0;
  float sustain_level_ = 0;
  float release_ = 0;
  float release_slope_ = 0;
  float level_ = 0;
  float release_start_ = 0;  /* initial level of release stage */
  int   sustain_steps_ = 0;  /* sustain smoothing */
  bool  params_changed_ = true;
  int   rate_ = 48000;

  enum class State { ATTACK, DECAY, SUSTAIN, RELEASE, DONE };

  State state_ = State::DONE;
  Shape shape_ = Shape::LINEAR;

  float a_ = 0;
  float b_ = 0;
  float c_ = 0;

  void
  init_abc (float time_s, float slope)
  {
    bool positive = slope > 0;
    slope = std::abs (slope);

    const float t1y = 0.5f + 0.25f * slope;

    a_ = slope * ( 1.0135809670870777f + slope * (-1.2970447050283254f + slope *   7.2390617313972063f));
    b_ = slope * (-5.8998946320566281f + slope * ( 5.7282487210570903f + slope * -15.525953208626062f));
    c_ = 1 - (t1y * a_ + b_) * t1y;

    if (!positive)
      {
        c_ += a_ + b_;
        b_ = -2 * a_ - b_;
      }

    const float time_factor = 1 / (rate_ * time_s);
    a_ *= time_factor;
    b_ *= time_factor;
    c_ *= time_factor;
  }

  void
  compute_slope_params (float seconds, float start_x, float end_x)
  {
    if (!params_changed_)
      return;

    int steps = std::max<int> (seconds * rate_, 1);

    if (shape_ == Shape::LINEAR)
      {
        // linear
        a_ = 0;
        b_ = 0;
        c_ = (end_x - start_x) / steps;
      }
    else if (shape_ == Shape::EXPONENTIAL)
      {
        /* exponential: true exponential decay doesn't ever reach zero;
         * therefore we need to fade out early
         */
        const double RATIO = (state_ == State::ATTACK) ? 0.2 : 0.001;

        const double f = -log ((RATIO + 1) / RATIO) / steps;
        double factor = exp (f);
        c_ = (end_x - RATIO * (start_x - end_x)) * (1 - factor);
        b_ = factor - 1;
        a_ = 0;
      }
    else if (shape_ == Shape::FLEXIBLE)
      {
        auto pos_time = [] (auto x) { return std::max (x, 0.0001f); /* 0.1ms */ };
        if (state_ == State::ATTACK)
          {
            init_abc (pos_time (attack_), attack_slope_);
          }
        else if (state_ == State::DECAY)
          {
            /* exact timing for linear decay slope */
            float stretch = 1 / std::max (1 - sustain_level_, 0.01f);
            init_abc (-pos_time (decay_ * stretch), decay_slope_);
          }
        else if (state_ == State::RELEASE)
          {
            init_abc (-pos_time (release_), release_slope_);

            /* stretch abc parameters to match release time */
            float l = std::max (release_start_, 0.01f);
            a_ /= l;
            c_ *= l;
          }
      }
    params_changed_ = false;
  }

public:
  void
  set_shape (Shape shape)
  {
    shape_ = shape;
    params_changed_ = true;
  }
  void
  set_attack (float f)
  {
    attack_ = f;
    params_changed_ = true;
  }
  void
  set_attack_slope (float f)
  {
    attack_slope_ = f;
    params_changed_ = true;
  }
  void
  set_decay (float f)
  {
    decay_ = f;
    params_changed_ = true;
  }
  void
  set_decay_slope (float f)
  {
    decay_slope_ = f;
    params_changed_ = true;
  }
  void
  set_sustain (float f)
  {
    sustain_level_ = f * 0.01f;
    params_changed_ = true;
  }
  void
  set_release (float f)
  {
    release_ = f;
    params_changed_ = true;
  }
  void
  set_release_slope (float f)
  {
    release_slope_ = f;
    params_changed_ = true;
  }
  void
  set_rate (int sample_rate)
  {
    rate_ = sample_rate;
    params_changed_ = true;
  }
  void
  start ()
  {
    level_          = 0;
    state_          = State::ATTACK;
    params_changed_ = true;
  }
  void
  stop()
  {
    state_          = State::RELEASE;
    release_start_  = level_;
    params_changed_ = true;
  }
private:
  template<State STATE, Shape SHAPE>
  void
  process (uint *iptr, float *samples, uint n_samples)
  {
    uint i = *iptr;

    while (i < n_samples)
      {
        samples[i++] = level_;

        if (SHAPE == Shape::FLEXIBLE)
          level_ += (a_ * level_ + b_) * level_ + c_;

        if (SHAPE == Shape::EXPONENTIAL)
          level_ += b_ * level_ + c_;

        if (SHAPE == Shape::LINEAR)
          level_ += c_;

        if (STATE == State::ATTACK && level_ > 1)
          {
            level_          = 1;
            state_          = State::DECAY;
            params_changed_ = true;
            break;
          }
        if (STATE == State::DECAY && level_ < sustain_level_)
          {
            state_          = State::SUSTAIN;
            level_          = sustain_level_;
            params_changed_ = true;
            break;
          }
        if (STATE == State::RELEASE && level_ < 1e-5)
          {
            state_ = State::DONE;
            level_ = 0;
            break;
          }
      }

    *iptr = i;
  }
  template<State STATE>
  void
  process (uint *iptr, float *samples, uint n_samples)
  {
    if (shape_ == Shape::LINEAR)
      process<STATE, Shape::LINEAR> (iptr, samples, n_samples);

    if (shape_ == Shape::EXPONENTIAL)
      process<STATE, Shape::EXPONENTIAL> (iptr, samples, n_samples);

    if (shape_ == Shape::FLEXIBLE)
      process<STATE, Shape::FLEXIBLE> (iptr, samples, n_samples);
  }
public:
  void
  process (float *samples, uint n_samples)
  {
    uint i = 0;
    if (state_ == State::ATTACK)
      {
        compute_slope_params (attack_, 0, 1);
        process<State::ATTACK> (&i, samples, n_samples);
      }
    if (state_ == State::DECAY)
      {
        compute_slope_params (decay_, 1, sustain_level_);
        process<State::DECAY> (&i, samples, n_samples);
      }
    if (state_ == State::RELEASE)
      {
        compute_slope_params (release_, release_start_, 0);
        process<State::RELEASE> (&i, samples, n_samples);
      }
    if (state_ == State::SUSTAIN)
      {
        if (params_changed_)
          {
            if (std::abs (sustain_level_ - level_) > 1e-5)
              {
                sustain_steps_ = std::max<int> (0.020f * rate_, 1);
                c_ = (sustain_level_ - level_) / sustain_steps_;
              }
            else
              {
                sustain_steps_ = 0;
              }
            params_changed_ = false;
          }
        while (sustain_steps_ && i < n_samples) /* sustain smoothing */
          {
            samples[i++] = level_;
            level_ += c_;
            sustain_steps_--;
            if (sustain_steps_ == 0)
              level_ = sustain_level_;
          }
        while (i < n_samples)
          samples[i++] = level_;
      }
    if (state_ == State::DONE)
      {
        while (i < n_samples)
          samples[i++] = 0;
      }
  }
  bool
  is_constant() const
  {
    if (state_ == State::SUSTAIN)
      {
        return !params_changed_ && sustain_steps_ == 0;
      }
    return state_ == State::DONE;
  }
  bool
  done() const
  {
    return state_ == State::DONE;
  }
};

}
