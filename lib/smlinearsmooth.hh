/*
 * liquidsfz - sfz sampler
 *
 * Copyright (C) 2019  Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SPECTMORPH_LINEAR_SMOOTH_HH
#define SPECTMORPH_LINEAR_SMOOTH_HH

namespace SpectMorph
{

class LinearSmooth
{
  float value_ = 0;
  float linear_value_ = 0;
  float linear_step_ = 0;
  uint  total_steps_ = 1;
  uint  steps_ = 0;
public:
  void
  reset (uint rate, float time)
  {
    total_steps_ = std::max<int> (rate * time, 1);
  }
  void
  set (float new_value, bool now = false)
  {
    if (now)
      {
        steps_ = 0;
        value_ = new_value;
      }
    else if (new_value != value_)
      {
        if (!steps_)
          linear_value_ = value_;

        linear_step_ = (new_value - linear_value_) / total_steps_;
        steps_ = total_steps_;
        value_ = new_value;
      }
  }
  float
  get_next()
  {
    if (!steps_)
      return value_;
    else
      {
        steps_--;
        linear_value_ += linear_step_;
        return linear_value_;
      }
  }
  bool
  is_constant()
  {
    return steps_ == 0;
  }
};

}

#endif /* SPECTMOPRH_FILTER_ENVELOPE_HH */
