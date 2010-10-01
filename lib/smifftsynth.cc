#include "smifftsynth.hh"
#include "smmath.hh"
#include "smfft.hh"
#include <bse/gslfft.h>
#include <assert.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

IFFTSynth::IFFTSynth (size_t block_size, double mix_freq) :
  block_size (block_size),
  mix_freq (mix_freq)
{
  zero_padding = 256;

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
  win_trans.resize (zero_padding * 12);  /* > zero_padding * range * 2 */
  win_trans_center = zero_padding * 6;
  for (size_t i = 0; i < win_trans.size(); i++)
    {
      int pos = i - win_trans_center;
      assert (abs (pos * 2) < wspectrum.size());
      win_trans[i] = wspectrum[abs (pos * 2)];
    }
}

void
IFFTSynth::render_partial (float *buffer, double mf_freq, double mag, double phase)
{
  const int range = 4;

  // rotation for initial phase; scaling for magnitude
  const double phase_rcmag = 0.5 * mag * cos (-phase);
  const double phase_rsmag = 0.5 * mag * sin (-phase);

  const double freq = mf_freq / mix_freq * block_size;
  int ibin = freq;
  const double frac = freq - ibin;
  const double qfreq = ibin + int (frac * zero_padding) * (1. / zero_padding);
  int index = -range * zero_padding - frac * zero_padding;
  float *sp = buffer + 2 * (ibin - range);

  //  mf_qfreq = qfreq / block_size * mix_freq;

  index += win_trans_center;
  for (int i = 0; i <= 2 * range; i++)
    {
      const double wmag = win_trans[index];
      *sp++ += phase_rcmag * wmag;
      *sp++ += phase_rsmag * wmag;
      index += zero_padding;
    }
}

void
IFFTSynth::get_samples (const float *buffer, float *samples, const float *window)
{
  float *fft_out = FFT::new_array_float (block_size);    // FIXME: keep this

  FFT::fftsr_float (block_size, const_cast<float *> (buffer), fft_out);

  memcpy (samples, &fft_out[block_size / 2], sizeof (float) * block_size / 2);
  memcpy (&samples[block_size / 2], fft_out, sizeof (float) * block_size / 2);

  for (size_t i = 0; i < block_size; i++)
    samples[i] *= window[i] / window_blackman_harris_92 (2.0 * i / block_size - 1.0);

  FFT::free_array_float (fft_out);
}
