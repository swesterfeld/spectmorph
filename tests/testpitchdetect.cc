// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smwavdata.hh"
#include "smmath.hh"
#include "smfft.hh"

#include <cassert>

using namespace SpectMorph;

using std::vector;

struct SineDetectPartial
{
  double freq  = 0;
  double mag   = 0;
  double phase = 0;
};

namespace
{

class QInterpolator
{
  double a, b, c;

public:
  QInterpolator (double y1, double y2, double y3)
  {
    a = (y1 + y3 - 2*y2) / 2;
    b = (y3 - y1) / 2;
    c = y2;
  }
  double
  eval (double x)
  {
    return a * x * x + b * x + c;
  }
  double
  x_max()
  {
    return -b / (2 * a);
  }
};

}

vector<SineDetectPartial>
sine_detect (double mix_freq, const vector<float>& signal)
{
  assert (signal.size() % 2 != 0);

  /* possible improvements for this code
   *
   *  - could eliminate the sines to produce residual signal (spectral subtract)
   */
  vector<SineDetectPartial> partials;

  constexpr double MIN_PADDING = 4;

  size_t padded_length = 2;
  while (signal.size() * MIN_PADDING >= padded_length)
    padded_length *= 2;

  vector<float> padded_signal;
  float window_weight = 0;
  for (size_t i = 0; i < signal.size(); i++)
    {
      const float w = window_cos ((i - signal.size() * 0.5) / (signal.size() * 0.5));
      window_weight += w;
      padded_signal.push_back (signal[i] * w);
    }
  padded_signal.resize (padded_length);

  /* create odd/centered windowed input signal */
  vector<float> odd_centered (padded_length);
  for (size_t i = 0; i < padded_signal.size(); i++)
    odd_centered[i] = padded_signal[(i + signal.size() / 2) % odd_centered.size()];

  vector<float> fft_values (odd_centered.size());
  FFT::fftar_float (odd_centered.size(), odd_centered.data(), fft_values.data());

  vector<float> mag_values;
  for (size_t i = 0; i < fft_values.size(); i += 2)
    mag_values.push_back (sqrt (fft_values[i] * fft_values[i] + fft_values[i + 1] * fft_values[i + 1]));

  for (size_t x = 1; x + 2 < mag_values.size(); x++)
    {
      /* check for peaks
       *  - single peak : magnitude of the middle value is larger than
       *                  the magnitude of the left and right neighbour
       *  - double peak : two values in the spectrum have equal magnitude,
         *                this must be larger than left and right neighbour
       */
      const auto [m1, m2, m3, m4] = std::tie (mag_values[x - 1], mag_values[x], mag_values[x + 1],  mag_values[x + 2]);
      if ((m1 < m2 && m2 > m3) || (m1 < m2 && m2 == m3 && m3 > m4))
        {
          size_t xs, xe;
          for (xs = x - 1; xs > 0 && mag_values[xs] < mag_values[xs + 1]; xs--);
          for (xe = x + 1; xe < (mag_values.size() - 1) && mag_values[xe] > mag_values[xe + 1]; xe++);

          const double normalized_peak_width = double (xe - xs) * signal.size() / padded_length;

          const double mag1 = db_from_factor (mag_values[x - 1], -100);
          const double mag2 = db_from_factor (mag_values[x], -100);
          const double mag3 = db_from_factor (mag_values[x + 1], -100);
          QInterpolator mag_interp (mag1, mag2, mag3);
          double x_max = mag_interp.x_max();
          double peak_mag_db = mag_interp.eval (x_max);
          double peak_mag = db_to_factor (peak_mag_db) * (2 / window_weight);

          if (peak_mag > 0.0001 && normalized_peak_width > 2.9)
            {
              SineDetectPartial partial;
              partial.freq = (x + x_max) * mix_freq / padded_length;
              partial.mag  = peak_mag;

              /* compute phase */
              int d = 2 * x;
              QInterpolator re_interp (fft_values[d-2], fft_values[d],   fft_values[d+2]);
              QInterpolator im_interp (fft_values[d-1], fft_values[d+1], fft_values[d+3]);
              const double re_mag = re_interp.eval (x_max);
              const double im_mag = im_interp.eval (x_max);
              double phase = atan2 (im_mag, re_mag) + 0.5 * M_PI;
              // correct for the odd-centered analysis
              phase -= (signal.size() - 1) / 2.0 / mix_freq * partial.freq * 2 * M_PI;
              partial.phase = phase;

              partials.push_back (partial);
              // printf ("%f %f %f\n", (x + x_max) * mix_freq / padded_length, peak_mag * (2 / window_weight), normalized_peak_width);
            }
        }
    }

  return partials;
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  assert (argc == 2);

  WavData wav_data;
  bool ok = wav_data.load (argv[1]);
  assert (ok);

  for (int ssz = 99; ssz < 8192; ssz = int (ssz * 1.2))
    {
      if (ssz % 2 == 0)
        ssz += 1;

      double snr_signal_power = 0;
      double snr_delta_power = 0;

      vector<float> window (ssz);
      for (size_t i = 0; i < window.size(); i++)
        {
          window[i] = window_cos ((i - window.size() * 0.5) / (window.size() * 0.5));
        }
      for (size_t offset = 0; offset + ssz < wav_data.samples().size(); offset += 256)
        {
          vector<float> single_frame (wav_data.samples().begin() + offset, wav_data.samples().begin() + offset + ssz);
          //printf ("%zd\n", offset);
          auto partials = sine_detect (wav_data.mix_freq(), single_frame);

          for (size_t i = 0; i < single_frame.size(); i++)
            {
              double out = 0;
              for (size_t p = 0; p < partials.size(); p++)
                {
                  out += sin (i * partials[p].freq * 2 * M_PI / wav_data.mix_freq() + partials[p].phase) * partials[p].mag;
                }
              double signal = single_frame[i] * window[i];
              out *= window[i];
              double delta = out - signal;
              snr_signal_power += single_frame[i] * single_frame[i];
              snr_delta_power += delta * delta;
              //sm_printf ("%zd %.10f %.10f %.10f #%zd\n", i, signal, out, signal - out, offset / 256);
            }
          //printf ("%f Hz - %f\n", p.freq, p.mag);
        }
        sm_printf ("%d %f\n", ssz, 10 * log10 (snr_signal_power / snr_delta_power));
    }
}
