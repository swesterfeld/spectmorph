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

#include <stdio.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <bse/bse.h>
#include <bse/bsemathsignal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <vector>
#include <complex>

#include "smfft.hh"
#include "smmain.hh"
#include "smmath.hh"
#include "smcwt.hh"
#include "smfftthread.hh"
#include <math.h>

using std::vector;
using std::complex;
using Birnet::AlignedArray;
using namespace SpectMorph;

float
value_scale (float value)
{
  return bse_db_from_factor (value, -200);
}

void
CWT::make_png (vector< vector<float> >& results)
{
  const int width = results[0].size(), height = results.size();
  GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, /* has_alpha */ false, 8, width, height);

  double max_value = 0.0;
  // compute magnitudes from FFT data and figure out max peak
  for (size_t n = 0; n < results.size(); n++)
    {
      vector<float>& data = results[n];
      for (size_t i = 0; i < data.size(); i++)
        {
	  max_value = std::max (double (value_scale (data[i])), max_value);
        }
    }
  printf ("max_value = %f\n", max_value);

  uint row_stride = gdk_pixbuf_get_rowstride (pixbuf);
  for (size_t n = 0; n < results.size(); n++)
    {
      guchar *p = gdk_pixbuf_get_pixels (pixbuf);
      vector<float>& data = results[results.size() - 1 - n];  // high frequencies above low frequencies
      for (size_t i = 0; i < data.size(); i++)
        {
          double value = MAX (value_scale (data[i]) - max_value + 96, 0);
          float f = value / 96;
          int y = n;
          p[row_stride * y] = f * 255;
          p[row_stride * y + 1] = f * 255;
          p[row_stride * y + 2] = f * 255;
          p += 3;
	}
    }
  GError *error = 0;
  gdk_pixbuf_save (pixbuf, "cwt.png", "png", &error, "compression", "0", NULL);
}

template<class T> void
get_morlet (double x, double freq, T *re, T *im)
{
  double width = 1000;
  double beta = freq * freq / width;
  double amp = exp (-x*x / 2.0 * beta);
  double s, c;
  double norm = sqrt (freq);
  sincos (2 * x * M_PI * freq, &s, &c);
  *re = amp * c * norm;
  *im = amp * s * norm;
}

static double
complex_abs (double re, double im)
{
  return sqrt (re * re + im * im);
}

vector< vector<float> >
CWT::analyze_slow (const vector<float>& signal, FFTThread *fft_thread)
{
  int width = 0;
  double mre = 0, mim = 0;
  get_morlet (0, 10. / 44100, &mre, &mim);
  double v0 = sqrt (mre * mre + mim * mim), v1;
  do
    {
      get_morlet (width++, 10. / 44100, &mre, &mim);
      v1 = sqrt (mre * mre + mim * mim) / v0;
      //printf ("%d %.17g\n", width, v1);
    }
  while (v1 > 0.9);
  //printf ("width = %d\n", width);
  //return 0;

  size_t EXTRA_SPACE = width;
  size_t fft_size = 1;
  while (signal.size() + EXTRA_SPACE > fft_size)
    fft_size *= 2;

  float *in = FFT::new_array_float (fft_size);
  float *in_fft = FFT::new_array_float (fft_size);

  float *morlet = FFT::new_array_float (fft_size * 2);
  float *morlet_fft = FFT::new_array_float (fft_size * 2);

  float *out = FFT::new_array_float (fft_size * 2);
  float *out_fft = FFT::new_array_float (fft_size * 2);

  for (size_t i = 0; i < fft_size; i++)
    {
      if (i < signal.size())
        in[i] = signal[i];
      else
        in[i] = 0;
    }
  FFT::fftar_float (fft_size, in, in_fft);

  vector< vector<float> > results;
  for (float freq = 10; freq < 22050; freq += 25)
    {
      for (size_t i = 0; i < fft_size; i++)
        {
          int x;
          if (i < fft_size / 2)
            x = i;
          else
            x = -(fft_size - i);
          //double re, im;
          //get_morlet (x, &re, &im);
          get_morlet (x, freq / 44100, &morlet[i * 2], &morlet[i * 2 + 1]);
          //printf ("%d %f %f %f\n", x, freq, morlet[i * 2], morlet[i * 2 + 1]);
        }
      zero_float_block (fft_size * 2, out_fft);

      FFT::fftac_float (fft_size, morlet, morlet_fft);
      for (size_t i = 0; i < fft_size / 2; i++)
        {
          double re = morlet_fft[i * 2];
          double im = morlet_fft[i * 2 + 1];
          complex<double> mval (re, im);
          //complex<double> mval = 1;
          complex<double> sval (in_fft[i * 2], in_fft[i * 2 + 1]);
          complex<double> msval = mval * sval;
          out_fft[2 * i] = msval.real();
          out_fft[2 * i + 1] = msval.imag();
          //printf ("%zd %.17g %.17g\n", i, mval.real(), mval.imag());
          //printf ("%zd %.17g %.17g\n", i, msval.real(), msval.imag());
        }
      vector<float> line;
      FFT::fftsc_float (fft_size, out_fft, out);
      for (size_t i = 0; i < signal.size(); i++)
        {
          if ((i & 15) == 0)
            line.push_back (complex_abs (out[i * 2], out [i * 2 + 1]));
          //printf ("%zd %.17g\n", i, complex_abs (out[i * 2], out[i * 2 + 1]));
          //printf ("%zd %.17g %.17g\n", i, out[i * 2], out[i * 2 + 1]);
        }
      results.push_back (line);
      signal_progress (freq / 22050.0);

      if (fft_thread && fft_thread->command_is_obsolete()) // abort if user changed params
        break;
    }
  FFT::free_array_float (morlet);
  FFT::free_array_float (morlet_fft);
  FFT::free_array_float (out);
  FFT::free_array_float (out_fft);
  FFT::free_array_float (in);
  FFT::free_array_float (in_fft);

  return results;
}

vector< vector<float> >
CWT::analyze (const vector<float>& asignal, FFTThread *fft_thread)
{
  vector< vector<float> > results;

  // pad data with zeros to make moving average filter work properly
  const int WIDTH = 100;
  const size_t ORDER = 7;
  const int PADDING = (ORDER + 1) * WIDTH;
  vector<float> signal (asignal.size() + PADDING * 2);
  AlignedArray<float,16> sin_values (signal.size()), cos_values (signal.size());
  vector<float> mod_signal_c (signal.size() * 2);
  vector<float> new_mod_signal_c (signal.size() * 2);
  std::copy (asignal.begin(), asignal.end(), signal.begin() + PADDING);

  for (float freq = 50; freq < 22050; freq += 25)
    {
      VectorSinParams vsp;
      vsp.mix_freq = 44100;
      vsp.freq = freq;
      vsp.phase = 0;
      vsp.mag = 1;
      vsp.mode = VectorSinParams::REPLACE;
      fast_vector_sincosf (vsp, &sin_values[0], &sin_values[sin_values.size()], &cos_values[0]);

      // modulation: multiply with complex exp(-j*w*t)
      for (size_t i = 0; i < signal.size(); i++)
        {
          mod_signal_c[i * 2]     = cos_values[i] * signal[i];   // real
          mod_signal_c[i * 2 + 1] = sin_values[i] * signal[i];   // imag
        }

      /* filter a few times with moving average filter -> approximates exp() window function */
      for (size_t n = 0; n < ORDER; n++)
        {
          float avg_re = 0, avg_im = 0;
          for (size_t i = WIDTH; i < signal.size() - WIDTH; i++)
            {
              avg_re += mod_signal_c[i * 2 + WIDTH * 2] - mod_signal_c[i * 2 - WIDTH * 2];
              avg_im += mod_signal_c[i * 2 + 1 + WIDTH * 2] - mod_signal_c[i * 2 + 1 - WIDTH * 2];
              new_mod_signal_c[i * 2] = avg_re;
              new_mod_signal_c[i * 2 + 1] = avg_im;
            }
          mod_signal_c.swap (new_mod_signal_c);
        }
      // demodulation: multiply with complex exp(j*w*t)
      vector<float> line;
      for (size_t i = PADDING; i < signal.size() - PADDING; i += 16)
        {
          const float d_r = cos_values[i];
          const float d_i = -sin_values[i];
          const float m_r = mod_signal_c[i * 2];
          const float m_i = mod_signal_c[i * 2 + 1];
          // complex mul:  d * m
          const float re = d_r * m_r - d_i * m_i;
          const float im = d_i * m_r + d_r * m_i;
          // abs (d * m)
          line.push_back (sqrtf (re * re + im * im));
        }
      results.push_back (line);
      signal_progress (freq / 22050.0);

      if (fft_thread && fft_thread->command_is_obsolete()) // abort if user changed params
        break;
    }
  return results;
}
