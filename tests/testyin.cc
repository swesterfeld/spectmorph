// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

using std::vector;

namespace
{

constexpr double sample_rate = 48000;

// Symmetric Gaussian low-pass: no phase shift in this offline experiment.
// At expected_f0 the amplitude response is about 0.97; at 4*expected_f0
// it is about 0.61. Higher-frequency interference is attenuated strongly.
// Filtering the audio also removes its contribution to the difference floor.
void
filter_for_pitch (vector<float>& samples, double expected_f0)
{
  if (samples.empty())
    return;
  const double pi = std::acos (-1.0);
  double sigma = sample_rate / (8 * pi * expected_f0);
  int radius = std::ceil (4 * sigma);
  vector<double> kernel (radius + 1);
  double norm = 0;
  for (int k = 0; k <= radius; k++)
    {
      kernel[k] = std::exp (-0.5 * (k / sigma) * (k / sigma));
      norm += (k == 0 ? 1 : 2) * kernel[k];
    }
  vector<float> filtered (samples.size());
  for (size_t i = 0; i < samples.size(); i++)
    {
      double value = kernel[0] * samples[i];
      for (int k = 1; k <= radius; k++)
        {
          // Extend endpoints with their nearest sample.
          size_t left = i >= size_t (k) ? i - k : 0;
          size_t right = size_t (k) < samples.size() - i ? i + k : samples.size() - 1;
          value += kernel[k] * (double (samples[left]) + samples[right]);
        }
      filtered[i] = value / norm;
    }
  samples.swap (filtered);
}

// CDF of the Beta(2, 18) prior on the YIN threshold, for x in [0, 1].
double
threshold_cdf (double x)
{
  return 1.0 - std::pow (1.0 - x, 18) * (1.0 + 18.0 * x);
}

struct PitchCandidate
{
  size_t lag;
  double probability;
};

vector<PitchCandidate>
pitch_candidates (const float *D1, size_t size, double& unvoiced_probability)
{
  vector<PitchCandidate> candidates;
  size_t deepest = 0;
  for (size_t lag = 1; lag + 1 < size; lag++)
    {
      if (D1[lag] < D1[lag - 1] && D1[lag] < D1[lag + 1])
        {
          candidates.push_back ({lag, 0});
          if (D1[lag] < D1[candidates[deepest].lag])
            deepest = candidates.size() - 1;
        }
    }
  unvoiced_probability = 0;
  const size_t n_thresholds = 100;
  const double no_trough_probability = 0.01;
  const double weight_decay = std::exp (-2.0);
  double previous_cdf = 0;
  for (size_t bin = 1; bin <= n_thresholds; bin++)
    {
      double threshold = double (bin) / n_thresholds;
      double cdf = threshold_cdf (threshold);
      double mass = cdf - previous_cdf;
      previous_cdf = cdf;

      // Boltzmann prior over eligible troughs in increasing lag order:
      // rank r has weight exp(-2*r), normalized separately per threshold.
      double weight_sum = 0, weight = 1;
      for (const auto& candidate : candidates)
        if (D1[candidate.lag] < threshold)
          {
            weight_sum += weight;
            weight *= weight_decay;
          }
      if (weight_sum > 0)
        {
          weight = 1;
          for (auto& candidate : candidates)
            if (D1[candidate.lag] < threshold)
              {
                candidate.probability += mass * weight / weight_sum;
                weight *= weight_decay;
              }
        }
      else if (!candidates.empty())
        {
          // Preserve a weak pitch hypothesis when no trough qualifies.
          candidates[deepest].probability += mass * no_trough_probability;
          unvoiced_probability += mass * (1 - no_trough_probability);
        }
      else
        unvoiced_probability += mass;
    }
  return candidates;
}

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
    if (a >= 0)
      return 0;
    double x = -b / (2 * a);
    return std::isfinite (x) && std::abs (x) <= 1 ? x : 0;
  }
};

}


int
main (int argc, char **argv)
{
  constexpr size_t win = 1500;
  constexpr size_t hop = win / 4;
  double expected_f0 = 0;
  if (argc != 1)
    {
      if (argc != 3 || std::strcmp (argv[1], "--expected-f0") != 0)
        {
          fprintf (stderr, "Usage: %s [--expected-f0 HZ] < samples.txt\n", argv[0]);
          return argc == 2 && std::strcmp (argv[1], "--help") == 0 ? 0 : 1;
        }
      char *end = nullptr;
      expected_f0 = std::strtod (argv[2], &end);
      if (end == argv[2] || *end || !std::isfinite (expected_f0)
          || expected_f0 < sample_rate / (win - 2) || expected_f0 >= sample_rate / 2)
        {
          fprintf (stderr, "Expected F0 must be finite and in [%.3f, %.0f) Hz.\n",
                   sample_rate / (win - 2), sample_rate / 2);
          return 1;
        }
    }
  vector<float> f;
  char buffer[1024];
  while (fgets (buffer, 1024, stdin))
    {
      f.push_back (atof (buffer));
    }

  if (expected_f0 > 0)
    filter_for_pitch (f, expected_f0);
  for (size_t i = 0; i + win + win < f.size(); i += hop)
    {
      float D[win];
      for (size_t delay = 0; delay < win; delay++)
        {
          D[delay] = 0;
          for (size_t j = 0; j < win; j++)
            {
              float d = f[i + j] - f[i + j + delay];
              D[delay] += d * d;
            }
        }
      float D1[win];
      D1[0] = 1;
      double sum = 0;
      for (size_t delay = 1; delay < win; delay++)
        {
          sum += D[delay];
          D1[delay] = sum > 0 ? D[delay] / (sum / delay) : 1;
        }
      double unvoiced_probability;
      auto candidates = pitch_candidates (D1, win, unvoiced_probability);
      for (const auto& candidate : candidates)
        {
          if (candidate.probability > 0)
            {
              size_t dmin = candidate.lag;
              QInterpolator interp (-D[dmin - 1], -D[dmin], -D[dmin + 1]);
              double pitch = sample_rate / (dmin + interp.x_max());
              // Output: frame sample offset, frequency in Hz, probability.
              printf ("%zu %.9g %.9g\n", i, pitch, candidate.probability);
            }
        }
      // Frequency zero denotes the mass left over for unvoiced frames.
      printf ("%zu 0 %.9g\n", i, unvoiced_probability);
    }
  return 0;
}
