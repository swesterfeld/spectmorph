// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"
#include "smfft.hh"
#include "smpitchdetect.hh"

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

// wraps phase in range [0:2*pi]
static double
normalize_phase (double phase)
{
  const double inv_2pi = 1.0 / (2.0 * M_PI);
  phase *= inv_2pi;
  phase -= floor (phase);
  return phase * (2.0 * M_PI);
}

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
  const auto fft_size = padded_length;

  float *odd_centered = FFT::new_array_float (fft_size);
  for (size_t i = 0; i < padded_signal.size(); i++)
    odd_centered[i] = padded_signal[(i + signal.size() / 2) % fft_size];

  float *fft_values = FFT::new_array_float (fft_size);
  FFT::fftar_float (fft_size, odd_centered, fft_values);

  vector<float> mag_values;
  for (size_t i = 0; i < fft_size; i += 2)
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
              partial.phase = normalize_phase (phase);

              partials.push_back (partial);
              // printf ("%f %f %f\n", (x + x_max) * mix_freq / padded_length, peak_mag * (2 / window_weight), normalized_peak_width);
            }
        }
    }
  FFT::free_array_float (odd_centered);
  FFT::free_array_float (fft_values);

  return partials;
}

std::pair<double, double>
static pitch_detect_twm (const vector<SineDetectPartial>& partials)
{
  if (partials.size() == 0)
    return std::make_pair (-1.0, -1.0);

  double A_max = 0;
  double f_max = 0;
  double best_partial_f = 0;
  for (auto p : partials)
    {
      if (A_max < p.mag)
        best_partial_f = p.freq;
      A_max = std::max (A_max, p.mag);
      f_max = std::max (f_max, p.freq);
    }

  auto get_error = [&] (double freq)
    {
      const double p = 0.5;
      const double q = 1.4;
      const double r = 0.5;
      const double rho = 0.33;

      double error_p2m = 0;

      const int n_p2m_freqs = std::max<int> (lrint (f_max / freq), 1); // ceil ?
      size_t best_index = 0;
      for (int n = 1; n <= n_p2m_freqs; n++)
        {
          double f_harm = n * freq;
          double best_diff = std::abs (partials[best_index].freq - f_harm);
          double new_best_diff;
          while (best_index + 1 < partials.size() && (new_best_diff = std::abs (partials[best_index + 1].freq - f_harm)) < best_diff)
            {
              best_diff = new_best_diff;
              best_index++;
            }
          double diff_f_pow = best_diff * pow (f_harm, -p);

          error_p2m += diff_f_pow + partials[best_index].mag / A_max * (q * diff_f_pow - r);
        }
      double error_m2p = 0;

      const int n_m2p_freqs = partials.size();
      for (int n = 0; n < n_m2p_freqs; n++)
        {
          double n_harmonic = std::max (round (partials[n].freq / freq), 1.0);
          double freq_distance = std::abs (partials[n].freq - n_harmonic * freq);
          double diff_f_pow = freq_distance * pow (partials[n].freq, -p);

          error_m2p += diff_f_pow + partials[n].mag / A_max * (q * diff_f_pow - r);
        }
      double error = error_p2m / n_p2m_freqs + error_m2p * rho / n_m2p_freqs;
      return error;
    };
  double best_e = 1e300;
  double best_f = 0;

  auto improve_estimate = [&] (double f)
    {
      double e = get_error (f);
      if (e < best_e)
        {
          best_e = e;
          best_f = f;
          return true;
        }
      else
        {
          return false;
        }
    };

  /* the fundamental frequency is very often one of the frequencies in the
   * partial list we have
   */
  for (auto p : partials)
    improve_estimate (p.freq);

  /* typically the loudest partial is an integer multiple of the fundamental
   * frequency, so this can be used in cases where the fundamental is missing
   * in the input partial list
   */
  for (int n = 1; n <= 64; n++)
    improve_estimate (best_partial_f / n);

  /* at this point we're already really close to a local minimum, typically
   * only a few cent away, so we try to do a few improvement steps to get
   * cent resolution for the pitch detection
   */
  const double cent_factor = 1.00057778950655;

  for (int it = 0; improve_estimate (best_f * cent_factor) && it < 100; it++);
  for (int it = 0; improve_estimate (best_f / cent_factor) && it < 100; it++);

  /* correct for possible octave error (however this is unlikely to happen) */
  for (auto f : { best_f / 4, best_f / 3, best_f / 2, best_f * 2, best_f * 3, best_f * 4})
    improve_estimate (f);

  return std::make_pair (best_f, best_e);
}

static double
note_to_freq (double note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

static double
detect_pitch_mono (const WavData& wav_data, std::function<bool (double)> kill_progress_function)
{
  assert (wav_data.n_channels() == 1);

  double best_snr = 0;
  vector<double> freqs;
  vector<vector<SineDetectPartial>> best_partials_vec;

  vector<int> window_size_ms { 5, 10, 20, 40, 80, 160 };
  for (size_t ws_index = 0; ws_index < window_size_ms.size(); ws_index++)
    {
      int ssz = window_size_ms[ws_index] * 0.001 * wav_data.mix_freq();
      if (ssz % 2 == 0)
        ssz += 1;

      double snr_signal_power = 0;
      double snr_delta_power = 0;
      vector<vector<SineDetectPartial>> partials_vec;

      vector<float> window (ssz);
      for (size_t i = 0; i < window.size(); i++)
        {
          window[i] = window_cos ((i - window.size() * 0.5) / (window.size() * 0.5));
        }

      size_t progress_frames_total = 0;
      for (size_t offset = 0; offset + ssz < wav_data.samples().size(); offset += ssz / 4)
        progress_frames_total++;

      size_t progress_frames_done = 0;
      for (size_t offset = 0; offset + ssz < wav_data.samples().size(); offset += ssz / 4)
        {
          vector<float> single_frame (wav_data.samples().begin() + offset, wav_data.samples().begin() + offset + ssz);
          //printf ("%zd\n", offset);
          auto partials = sine_detect (wav_data.mix_freq(), single_frame);
          partials_vec.push_back (partials);

          vector<float> out_frame (single_frame.size());
          for (size_t p = 0; p < partials.size(); p++)
            {
              VectorSinParams params;

              params.mix_freq = wav_data.mix_freq();
              params.freq     = partials[p].freq;
              params.phase    = partials[p].phase;
              params.mag      = partials[p].mag;
              params.mode     = VectorSinParams::ADD;

              fast_vector_sin (params, out_frame.begin(), out_frame.end());
            }
          for (size_t i = 0; i < single_frame.size(); i++)
            {
              double out = out_frame[i] * window[i];
              double signal = single_frame[i] * window[i];
              double delta = out - signal;
              snr_signal_power += signal * signal;
              snr_delta_power += delta * delta;
            }

          if (kill_progress_function && kill_progress_function ((ws_index + double (progress_frames_done++) / progress_frames_total) * 90.0 / window_size_ms.size()))
            return -1;
        }
      if (progress_frames_total > 0)
        {
          double snr = 10 * log10 (snr_signal_power / snr_delta_power);
          if (snr > best_snr)
            {
              best_snr = snr;
              best_partials_vec = partials_vec;
            }
        }
    }
  vector<double> mag_sums;
  for (size_t bpv_index = 0; bpv_index < best_partials_vec.size(); bpv_index++)
    {
      const auto& partials = best_partials_vec[bpv_index];

      double A_max = 0;
      double A_sum = 0;
      for (auto p : partials)
        {
          A_max = std::max (p.mag, A_max);
          A_sum += p.mag;
        }
      vector<SineDetectPartial> strong_partials;
      for (auto p : partials)
        if (p.mag / A_max > 0.01)
          strong_partials.push_back (p);

      auto [twm_freq, twm_err] = pitch_detect_twm (strong_partials);
      if (twm_freq > 0)
        {
          freqs.push_back (twm_freq);
          mag_sums.push_back (A_sum);
        }
      if (kill_progress_function && kill_progress_function (90 + 10.0 * bpv_index / best_partials_vec.size()))
        return -1;
    }

  auto get_best_note = [&] (double note_min, double note_max, double step)
    {
      double note_freq_min = note_to_freq (note_min);
      double note_freq_max = note_to_freq (note_max);

      double best_note = 0;
      double best_err = 1e300;
      for (double note = note_min; note < note_max; note += step)
        {
          double freq = note_to_freq (note);
          double ferr = 0;

          for (size_t i = 0; i < freqs.size(); i++)
            {
              if (freqs[i] >= note_freq_min && freqs[i] <= note_freq_max)
                ferr += std::abs (freqs[i] - freq) * mag_sums[i];
            }
          if (ferr < best_err)
            {
              best_err = ferr;
              best_note = note;
            }
        }
      return best_note;
    };

  double best_note = -1; /* return -1 if pitch detection fails */
  if (freqs.size())
    {
      double best_note_estimate = get_best_note (0, 128, 0.1);

      /* - improve estimate using finer grid
       * - assume final result is in +200/-200 cent of previous estimate
       * - exclude outliers (+200/-200 cent) from error computation
       */
      best_note = get_best_note (best_note_estimate - 2, best_note_estimate + 2, 0.01);
    }

  if (kill_progress_function)
    kill_progress_function (100);
  return best_note;
}

namespace SpectMorph
{

double
detect_pitch (const WavData& wav_data, std::function<bool (double)> kill_progress_function)
{
  if (wav_data.n_channels() == 1)
    {
      return detect_pitch_mono (wav_data, kill_progress_function);
    }
  else
    {
      const vector<float> in_samples = wav_data.samples();
      vector<float> flat_mono_samples;
      int n_channels = wav_data.n_channels();

      for (int ch = 0; ch < n_channels; ch++)
        {
          for (size_t i = ch; i < in_samples.size(); i += n_channels)
            flat_mono_samples.push_back (in_samples[i]);
        }

      WavData flat_wav_data (flat_mono_samples, 1, wav_data.mix_freq(), wav_data.bit_depth());
      return detect_pitch_mono (flat_wav_data, kill_progress_function);
    }
}

std::pair<double, double>
pitch_detect_twm_test (const vector<double>& freqs_mags) /* for unit tests */
{
  vector<SineDetectPartial> partials;
  for (size_t i = 0; i < freqs_mags.size(); i += 2)
    {
      SineDetectPartial p;
      p.freq = freqs_mags[i];
      p.mag  = freqs_mags[i + 1];
      partials.push_back (p);
    }
  return pitch_detect_twm (partials);
}

}
