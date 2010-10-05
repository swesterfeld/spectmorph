#include "smifftsynth.hh"
#include "smmath.hh"
#include "smfft.hh"
#include <bse/gslfft.h>
#include <assert.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;
using std::map;

struct SpectMorph::IFFTSynthTable
{
  std::vector<float> win_trans;
  int                win_trans_center;

  std::vector<float> win_bh92;
};

static map<size_t, IFFTSynthTable *> table_for_block_size;

IFFTSynth::IFFTSynth (size_t block_size, double mix_freq) :
  block_size (block_size),
  mix_freq (mix_freq)
{
  zero_padding = 256;

  table = table_for_block_size[block_size];
  if (!table)
    {
      table = new IFFTSynthTable();

      vector<double> win (block_size * zero_padding);
      vector<double> wspectrum (block_size * zero_padding);

      for (size_t i = 0; i < block_size; i++)
        {
          if (i < block_size / 2)
            win[i] = window_blackman_harris_92 (double (block_size / 2 - i) / block_size * 2 - 1.0);
          else
            win[win.size() - block_size + i] = window_blackman_harris_92 (double (i - block_size / 2) / block_size * 2 - 1.0);
        }

      gsl_power2_fftar (block_size * zero_padding, &win[0], &wspectrum[0]);

      // compute complete (symmetric) expanded window transform
      table->win_trans.resize (zero_padding * 12);  /* > zero_padding * range * 2 */
      table->win_trans_center = zero_padding * 6;
      for (size_t i = 0; i < table->win_trans.size(); i++)
        {
          int pos = i - table->win_trans_center;
          assert (abs (pos * 2) < wspectrum.size());
          table->win_trans[i] = wspectrum[abs (pos * 2)];
        }

      // store a blackman harris window
      table->win_bh92.resize (block_size);
      for (size_t i = 0; i < block_size; i++)
        table->win_bh92[i] = window_blackman_harris_92 (2.0 * i / block_size - 1.0);

      // we only need to do this once per block size (FIXME: not thread safe yet)
      table_for_block_size[block_size] = table;
    }
  fft_in = FFT::new_array_float (block_size);
  fft_out = FFT::new_array_float (block_size);
  freq256_factor = 1 / mix_freq * block_size * zero_padding;
}

IFFTSynth::~IFFTSynth()
{
  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
}

void
IFFTSynth::clear_partials()
{
  zero_float_block (block_size, fft_in);
}

void
IFFTSynth::render_partial (double mf_freq, double mag, double phase)
{
  const int range = 4;

  const int freq256 = mf_freq * freq256_factor;
  const int ibin = freq256 / 256;
  const double qfreq = freq256 / 256.0;
  int index = -range * zero_padding - (freq256 & 0xff);
  float *sp = fft_in + 2 * (ibin - range);

  // adjust phase to get the same output like vector sin (smmath.hh)
  const double mf_qfreq = qfreq / block_size * mix_freq;
  const double delta_phase = mf_qfreq / mix_freq * 2 * M_PI;
  const double phase_adjust = delta_phase * (block_size / 2) - M_PI / 2;

  // rotation for initial phase; scaling for magnitude
  const double phase_rcmag = 0.5 * mag * cos (-phase - phase_adjust);
  const double phase_rsmag = 0.5 * mag * sin (-phase - phase_adjust);
  if (ibin > range && 2 * (ibin + range) < block_size)
    {
      index += table->win_trans_center;
      for (int i = 0; i <= 2 * range; i++)
        {
          const double wmag = table->win_trans[index];
          *sp++ += phase_rcmag * wmag;
          *sp++ += phase_rsmag * wmag;
          index += zero_padding;
        }
    }
  else
    {
      index += table->win_trans_center;
      for (int i = -range; i <= range; i++)
        {
          const double wmag = table->win_trans[index];
          if ((ibin + i) < 0)
            {
              fft_in[-(ibin + i) * 2] += phase_rcmag * wmag;
              fft_in[-(ibin + i) * 2 + 1] -= phase_rsmag * wmag;
            }
          else if ((ibin + i) == 0)
            {
              fft_in[0] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) == block_size)
            {
              fft_in[1] += 2 * phase_rcmag * wmag;
            }
          else if (2 * (ibin + i) > block_size)
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
          index += zero_padding;
        }
    }
}

void
IFFTSynth::get_samples (float *samples, const float *window)
{
  FFT::fftsr_float (block_size, fft_in, fft_out);

  memcpy (samples, &fft_out[block_size / 2], sizeof (float) * block_size / 2);
  memcpy (&samples[block_size / 2], fft_out, sizeof (float) * block_size / 2);

  for (size_t i = 0; i < block_size; i++)
    samples[i] *= window[i] / table->win_bh92[i];
}

double
IFFTSynth::quantized_freq (double mf_freq)
{
  const double freq = mf_freq / mix_freq * block_size;
  int ibin = freq;
  const double frac = freq - ibin;
  const double qfreq = ibin + int (frac * zero_padding) * (1. / zero_padding);
  const double mf_qfreq = qfreq / block_size * mix_freq;

  return mf_qfreq;
}
