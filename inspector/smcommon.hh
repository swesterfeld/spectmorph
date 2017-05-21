// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMMON_HH
#define SPECTMORPH_COMMON_HH

#include <vector>

namespace SpectMorph
{

enum TransformType
{
  SM_TRANSFORM_NONE = 0,
  SM_TRANSFORM_FFT = 1,
  SM_TRANSFORM_CWT = 2,
  SM_TRANSFORM_LPC = 3
};

enum CWTMode
{
  SM_CWT_MODE_NONE  = 0,
  SM_CWT_MODE_VTIME = 1,
  SM_CWT_MODE_CTIME = 2
};

enum class FFTWindow
{
  HANNING,
  HAMMING,
  BLACKMAN,
  BLACKMAN_HARRIS_92
};

struct AnalysisParams
{
  TransformType transform_type;

  double        frame_size_ms; /* FFT */
  double        frame_step_ms; /* FFT */
  FFTWindow     fft_window;    /* FFT */

  CWTMode       cwt_mode;
  double        cwt_freq_resolution;  /* CWT */
  double        cwt_time_resolution;  /* CWT */
};

struct FFTResult
{
  std::vector<float> mags;
};

}

#endif
