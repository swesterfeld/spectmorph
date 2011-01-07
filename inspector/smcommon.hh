/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTMORPH_COMMON_HH
#define SPECTMORPH_COMMON_HH

namespace SpectMorph
{

enum TransformType
{
  SM_TRANSFORM_NONE = 0,
  SM_TRANSFORM_FFT = 1,
  SM_TRANSFORM_CWT = 2
};

struct AnalysisParams
{
  TransformType transform_type;

  double        frame_size_ms; /* FFT */
  double        frame_step_ms; /* FFT */

  double        cwt_freq_resolution;  /* CWT */
  double        cwt_time_resolution;  /* CWT */
};

struct FFTResult
{
  std::vector<float> mags;
};

}

#endif
