// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_IFFT_SYNTH_HH
#define SPECTMORPH_IFFT_SYNTH_HH

#include <sys/types.h>
#include <vector>

#include "smmath.hh"

namespace SpectMorph {

struct IFFTSynthTable;

class IFFTSynth
{
  IFFTSynthTable    *table;

  int                zero_padding;
  size_t             block_size;
  double             mix_freq;
  double             freq256_factor;
  double             mag_norm;

  float             *fft_in;
  float             *fft_out;
  float             *win_scale;

  enum { 
    SIN_TABLE_SIZE = 4096,
    SIN_TABLE_MASK = 4095
  };

  static std::vector<float> sin_table;

public:
  enum WindowType { WIN_BLACKMAN_HARRIS_92, WIN_HANNING };
  enum OutputMode { REPLACE, ADD };

  IFFTSynth (size_t block_size, double mix_freq, WindowType win_type);
  ~IFFTSynth();

  void
  clear_partials()
  {
    zero_float_block (block_size, fft_in);
  }

  float*
  fft_buffer()
  {
    return fft_in;
  }

  inline void render_partial (double freq, double mag, double phase);
  void get_samples (float *samples, OutputMode output_mode = REPLACE);

  double quantized_freq (double freq);
};

struct IFFTSynthTable
{
  std::vector<float> win_trans;

  float             *win_scale;
};

inline void
IFFTSynth::render_partial (double mf_freq, double mag, double phase)
{
  const int range = 4;

  const int freq256 = sm_round_positive (mf_freq * freq256_factor);
  const int ibin = freq256 >> 8;
  float *sp = fft_in + 2 * (ibin - range);
  const float *wmag_p = &table->win_trans[(freq256 & 0xff) * (range * 2 + 1)];

  const float nmag = mag * mag_norm;

  // rotation for initial phase; scaling for magnitude

  /* the following block computes sincos (phase + phase_adjust) */
  int iarg = sm_round_positive (phase * (SIN_TABLE_SIZE / (2 * M_PI)));

  // adjust phase to get the same output like vector sin (smmath.hh)
  // phase_adjust = freq256 * (M_PI / 256.0) - M_PI / 2;
  int iphase_adjust = freq256 * SIN_TABLE_SIZE / 512 + (SIN_TABLE_SIZE - SIN_TABLE_SIZE / 4);
  iarg += iphase_adjust;

  const float phase_rsmag = sin_table [iarg & SIN_TABLE_MASK] * nmag;
  iarg += SIN_TABLE_SIZE / 4;
  const float phase_rcmag = sin_table [iarg & SIN_TABLE_MASK] * nmag;

  /* compute FFT spectrum modifications */
  if (ibin > range && 2 * (ibin + range) < static_cast<int> (block_size))
    {
      for (int i = 0; i <= 2 * range; i++)
        {
          const float wmag = wmag_p[i];
          *sp++ += phase_rcmag * wmag;
          *sp++ += phase_rsmag * wmag;
        }
    }
  else
    {
      wmag_p += range; // allow negative addressing
      for (int i = -range; i <= range; i++)
        {
          const float wmag = wmag_p[i];
          if ((ibin + i) < 0)
            {
              fft_in[-(ibin + i) * 2] += phase_rcmag * wmag;
              fft_in[-(ibin + i) * 2 + 1] -= phase_rsmag * wmag;
            }
          else if ((ibin + i) == 0)
            {
              fft_in[0] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) == static_cast<int> (block_size))
            {
              fft_in[1] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) > static_cast<int> (block_size))
            {
              int p = block_size - (2 * (ibin + i) - block_size);

              fft_in[p] += phase_rcmag * wmag;
              fft_in[p + 1] -= phase_rsmag * wmag;
            }
          else // no corner case
            {
              fft_in[(ibin + i) * 2] += phase_rcmag * wmag;
              fft_in[(ibin + i) * 2 + 1] += phase_rsmag * wmag;
            }
        }
    }
}

}

#endif
