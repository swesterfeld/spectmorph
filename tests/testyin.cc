// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <cassert>
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <clocale>

#include "smencoder.hh"
#include "smmain.hh"

using std::vector;

namespace
{

constexpr double sample_rate = 48000;

// Symmetric Gaussian low-pass: no phase shift in this offline experiment.
// At expected_f0 the amplitude response is about 0.97; at 4*expected_f0
// it is about 0.61. Higher-frequency interference is attenuated strongly.
// Filtering the audio also removes its contribution to the difference floor.
bool
filter_for_pitch (vector<float>& samples, double sample_rate, double expected_f0,
                  const std::function<bool()>& cancelled)
{
  if (samples.empty())
    return true;
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
      if (i % 1024 == 0 && cancelled && cancelled())
        return false;
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
  return true;
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
pitch_candidates (const float *D1, size_t min_lag, size_t max_lag, double& unvoiced_probability)
{
  vector<PitchCandidate> candidates;
  size_t deepest = 0;
  for (size_t lag = min_lag; lag <= max_lag; lag++)
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

// Offline prototype: keep the encoder-facing API independent of the test CLI.
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
static_assert (false, "PYin is not fast-math safe: fix finite checks and Viterbi infinity handling before moving to lib/ with fast-math enabled.");
#endif

class PYin
{
public:
  struct Params
  {
    double mix_freq = 0;
    double min_freq = 0;
    double max_freq = 0;
    double expected_f0 = 0;
    size_t window_size = 0; // terms in the difference sum, not total support
  };

  struct Frame
  {
    double freq = 0;                 // Hz, or zero for decoded unvoiced
    bool voiced = false;            // Viterbi decision
    double voiced_probability = 0; // local evidence, not path confidence
  };

  struct Candidate
  {
    double freq;
    double probability;
  };

  struct Observation
  {
    vector<Candidate> candidates;
    double unvoiced_probability = 1;
  };

  struct Difference
  {
    vector<float> D;
    vector<float> D1;
  };

  explicit PYin (const Params& params);

  // Centers are strictly increasing sample offsets in the original mono input.
  // Negative/out-of-input centers use zero padding. Empty centers succeed.
  // Returns false on cancellation; output and diagnostics are then empty.
  // Valid parameters, strictly increasing centers and finite samples are contracts.
  bool analyze (const vector<float>& samples, const vector<int64_t>& centers,
                vector<Frame>& output, const std::function<bool()>& cancelled = {});

  // Optional diagnostics for this experiment, valid until the next analyze().
  const vector<Observation>& observations() const { return observations_; }

  // Single-center diagnostic, with the same filtering/padding as analyze().
  // Does not run Viterbi or replace observations from a previous analysis.
  Difference difference (const vector<float>& samples, int64_t center) const;

private:
  Params params_;
  size_t min_lag_, max_lag_, support_size_;
  vector<Observation> observations_;

  Difference compute_difference (const vector<float>& samples, int64_t center) const;
  Observation observe (const vector<float>& samples, int64_t center) const;
  bool decode (const vector<Observation>& observations, const vector<int64_t>& centers,
               vector<Frame>& output, const std::function<bool()>& cancelled) const;
};

PYin::PYin (const Params& params) : params_ (params)
{
  assert (std::isfinite (params.mix_freq) && params.mix_freq > 0);
  assert (std::isfinite (params.min_freq) && params.min_freq > 0);
  assert (std::isfinite (params.max_freq) && params.max_freq > params.min_freq);
  assert (params.max_freq <= params.mix_freq / 2);
  assert (std::isfinite (params.expected_f0) && params.expected_f0 >= 0);
  assert (params.expected_f0 < params.mix_freq / 2);
  assert (params.window_size > 0);
  assert (params.window_size <= size_t (std::numeric_limits<int>::max() / 2));
  assert (params.mix_freq / params.min_freq <= std::numeric_limits<int>::max() / 2);
  assert (params.expected_f0 == 0
          || params.mix_freq / params.expected_f0 <= std::numeric_limits<int>::max() / 2);

  min_lag_ = std::floor (params.mix_freq / params.max_freq);
  max_lag_ = std::ceil (params.mix_freq / params.min_freq);
  // Includes the right neighbor needed for interpolation at max_lag_.
  support_size_ = params.window_size + max_lag_ + 1;
}

PYin::Difference
PYin::difference (const vector<float>& samples, int64_t center) const
{
  for (auto sample : samples)
    assert (std::isfinite (sample));
  if (params_.expected_f0 > 0)
    {
      vector<float> filtered = samples;
      filter_for_pitch (filtered, params_.mix_freq, params_.expected_f0, {});
      return compute_difference (filtered, center);
    }
  return compute_difference (samples, center);
}

PYin::Difference
PYin::compute_difference (const vector<float>& samples, int64_t center) const
{
  vector<float> block (support_size_, 0);
  // The center is the middle sample (left of the midpoint for even support).
  const int64_t left = (support_size_ - 1) / 2;
  for (size_t j = 0; j < block.size(); j++)
    {
      // Avoid signed overflow even for centers at the limits of int64_t.
      const int64_t offset = int64_t (j) - left;
      if ((offset < 0 && center < std::numeric_limits<int64_t>::min() - offset)
          || (offset > 0 && center > std::numeric_limits<int64_t>::max() - offset))
        continue;
      const int64_t index = center + offset;
      if (index >= 0 && uint64_t (index) < samples.size())
        block[j] = samples[index];
    }

  Difference result;
  result.D.resize (max_lag_ + 2);
  result.D1.resize (max_lag_ + 2);
  auto& D = result.D;
  auto& D1 = result.D1;
  for (size_t lag = 1; lag < D.size(); lag++)
    for (size_t j = 0; j < params_.window_size; j++)
      {
        float d = block[j] - block[j + lag];
        D[lag] += d * d;
      }
  D1[0] = 1;
  double sum = 0;
  for (size_t lag = 1; lag < D.size(); lag++)
    {
      sum += D[lag];
      D1[lag] = sum > 0 ? D[lag] / (sum / lag) : 1;
    }
  return result;
}

PYin::Observation
PYin::observe (const vector<float>& samples, int64_t center) const
{
  const auto curves = compute_difference (samples, center);
  const auto& D = curves.D;
  const auto& D1 = curves.D1;
  Observation result;
  auto candidates = pitch_candidates (D1.data(), min_lag_, max_lag_, result.unvoiced_probability);
  for (const auto& candidate : candidates)
    {
      if (candidate.probability == 0)
        continue;
      size_t lag = candidate.lag;
      QInterpolator interp (-D[lag - 1], -D[lag], -D[lag + 1]);
      double freq = params_.mix_freq / (lag + interp.x_max());
      if (freq >= params_.min_freq && freq <= params_.max_freq)
        result.candidates.push_back ({freq, candidate.probability});
      else
        result.unvoiced_probability += candidate.probability;
    }
  std::sort (result.candidates.begin(), result.candidates.end(),
             [] (const Candidate& a, const Candidate& b) { return a.freq < b.freq; });
  result.unvoiced_probability = std::min (1.0, result.unvoiced_probability);
  return result;
}

bool
PYin::decode (const vector<Observation>& observations, const vector<int64_t>& centers,
              vector<Frame>& output, const std::function<bool()>& cancelled) const
{
  // 10-cent pitch bins, each with a voiced and an unvoiced state. Unvoiced
  // states retain pitch continuity through gaps. Work in log probabilities.
  const double bins_per_octave = 120;
  const int bins = 1 + std::floor (bins_per_octave * std::log2 (params_.max_freq / params_.min_freq));
  const int states = 2 * bins;
  const double neg_inf = -std::numeric_limits<double>::infinity();
  auto log_prob = [] (double p) { return std::log (std::max (p, 1e-300)); };
  vector<double> previous (states, -std::log (states)), score (states), emission (states);
  vector<vector<int>> back (centers.size(), vector<int> (states));
  vector<Frame> frames (centers.size());

  for (size_t t = 0; t < centers.size(); t++)
    {
      if (cancelled && cancelled())
        return false;
      const auto& observation = observations[t];
      std::fill (emission.begin(), emission.begin() + bins, 0);
      std::fill (emission.begin() + bins, emission.end(), observation.unvoiced_probability / bins);
      for (const auto& candidate : observation.candidates)
        {
          int bin = std::lround (bins_per_octave * std::log2 (candidate.freq / params_.min_freq));
          bin = std::max (0, std::min (bins - 1, bin));
          emission[bin] += candidate.probability;
        }
      frames[t].voiced_probability = 1 - observation.unvoiced_probability;
      if (t == 0)
        {
          for (int state = 0; state < states; state++)
            score[state] = previous[state] + log_prob (emission[state]);
        }
      else
        {
          // Actual elapsed time, including irregular center spacing.
          double dt = (static_cast<long double> (centers[t]) - centers[t - 1]) / params_.mix_freq;
          double radius = std::floor (35.92 * dt * bins_per_octave);
          radius = std::min (radius, double (bins - 1));
          int reach = radius;
          // A symmetric continuous-time voicing model: 1% switching at 10ms.
          double change = -0.5 * std::expm1 (std::log (0.98) * dt / 0.01);
          double log_change = log_prob (change), log_stay = std::log1p (-change);
          vector<double> log_weight (reach + 1), log_norm (bins);
          for (int d = 0; d <= reach; d++)
            log_weight[d] = std::log (radius + 1 - d);
          for (int src = 0; src < bins; src++)
            {
              double l = std::min (src, reach), r = std::min (bins - 1 - src, reach);
              log_norm[src] = std::log ((l + r + 1) * (radius + 1)
                                       - (l * (l + 1) + r * (r + 1)) / 2);
            }
          for (int dst = 0; dst < bins; dst++)
            {
              double best[2] = {neg_inf, neg_inf};
              int source[2] = {0, bins};
              for (int src = std::max (0, dst - reach); src <= std::min (bins - 1, dst + reach); src++)
                {
                  double transition = log_weight[std::abs (src - dst)] - log_norm[src];
                  for (int voice = 0; voice < 2; voice++)
                    {
                      int state = src + voice * bins;
                      double value = previous[state] + transition;
                      if (value > best[voice])
                        {
                          best[voice] = value;
                          source[voice] = state;
                        }
                    }
                }
              for (int voice = 0; voice < 2; voice++)
                {
                  int state = dst + voice * bins;
                  bool stay = best[voice] + log_stay >= best[1 - voice] + log_change;
                  score[state] = (stay ? best[voice] + log_stay : best[1 - voice] + log_change)
                                 + log_prob (emission[state]);
                  back[t][state] = source[stay ? voice : 1 - voice];
                }
            }
        }
      double maximum = *std::max_element (score.begin(), score.end());
      for (auto& value : score)
        value -= maximum;
      previous.swap (score);
    }

  int state = std::max_element (previous.begin(), previous.end()) - previous.begin();
  for (size_t t = centers.size(); t-- > 0;)
    {
      if (cancelled && cancelled())
        return false;
      frames[t].voiced = state < bins;
      frames[t].freq = frames[t].voiced ? params_.min_freq * std::exp2 (state / bins_per_octave) : 0;
      state = back[t][state];
    }
  output.swap (frames);
  return true;
}

bool
PYin::analyze (const vector<float>& samples, const vector<int64_t>& centers,
               vector<Frame>& output, const std::function<bool()>& cancelled)
{
  output.clear();
  observations_.clear();
  for (size_t i = 1; i < centers.size(); i++)
    assert (centers[i] > centers[i - 1]);
  if (cancelled && cancelled())
    return false;
  if (centers.empty())
    return true;
  for (size_t i = 0; i < samples.size(); i++)
    {
      if (i % 65536 == 0 && cancelled && cancelled())
        return false;
      assert (std::isfinite (samples[i]));
    }
  vector<float> filtered;
  const vector<float> *input = &samples;
  if (params_.expected_f0 > 0)
    {
      filtered = samples;
      if (!filter_for_pitch (filtered, params_.mix_freq, params_.expected_f0, cancelled))
        return false;
      input = &filtered;
    }
  vector<Observation> observations;
  observations.reserve (centers.size());
  for (auto center : centers)
    {
      if (cancelled && cancelled())
        return false;
      observations.push_back (observe (*input, center));
    }
  if (!decode (observations, centers, output, cancelled))
    return false;
  observations_.swap (observations);
  return true;
}

bool
write_encoder_pitch (const vector<float>& samples, double fundamental, const char *filename)
{
  std::ofstream out (filename);
  if (!out)
    {
      fprintf (stderr, "cannot open encoder output file\n");
      return false;
    }
  // Encoder/FFT infrastructure requires SpectMorph initialization.
  int argc = 0;
  char **argv = nullptr;
  SpectMorph::Main context (&argc, &argv);
  std::setlocale (LC_NUMERIC, "C");
  SpectMorph::WavData wav (samples, 1, sample_rate, 32);
  SpectMorph::EncoderParams params;
  params.setup_params (wav, fundamental);
  SpectMorph::Encoder encoder (params);
  if (!encoder.encode (wav, 0, 1, /* attack */ false, /* track_sines */ true))
    {
      fprintf (stderr, "encoder analysis failed\n");
      return false;
    }

  // Mirror compute_stft's leading padding, converting each window center back
  // to the original input's coordinates. env_f0 is a ratio, not Hz.
  const int64_t padding = params.frame_size - params.frame_step / 2;
  out << "# center_sample frequency_hz env_f0\n" << std::setprecision (12);
  for (size_t i = 0; i < encoder.audio_blocks.size(); i++)
    {
      int64_t center = int64_t (i * params.frame_step) + int64_t (params.frame_size / 2) - padding;
      double env_f0 = encoder.audio_blocks[i].env_f0;
      out << center << ' ' << env_f0 * fundamental << ' ' << env_f0 << '\n';
    }
  out.close();
  if (!out)
    {
      fprintf (stderr, "cannot write encoder output file\n");
      return false;
    }
  return true;
}

}

int
main (int argc, char **argv)
{
  constexpr size_t win = 1500;
  constexpr size_t hop = win / 4;
  PYin::Params params;
  params.mix_freq = sample_rate;
  params.min_freq = sample_rate / (win - 2);
  params.max_freq = sample_rate / 2;
  params.window_size = win;
  bool track = false;
  bool dump_d = false;
  int64_t dump_center = 0;
  const char *encoder_output = nullptr;
  double encoder_f0 = 440;
  for (int arg = 1; arg < argc; arg++)
    {
      if (std::strcmp (argv[arg], "--track") == 0)
        track = true;
      else if (std::strcmp (argv[arg], "--encoder-output") == 0 && arg + 1 < argc)
        encoder_output = argv[++arg];
      else if (std::strcmp (argv[arg], "--encoder-f0") == 0 && arg + 1 < argc)
        {
          const char *value = argv[++arg];
          char *end = nullptr;
          encoder_f0 = std::strtod (value, &end);
          if (end == value || *end || !std::isfinite (encoder_f0)
              || encoder_f0 < params.min_freq || encoder_f0 >= params.max_freq)
            {
              fprintf (stderr, "Encoder F0 must be in [%.3f, %.0f) Hz.\n", params.min_freq, params.max_freq);
              return 1;
            }
        }
      else if (std::strcmp (argv[arg], "--dump-d") == 0 && arg + 1 < argc)
        {
          const char *value = argv[++arg];
          char *end = nullptr;
          errno = 0;
          long long center = std::strtoll (value, &end, 10);
          if (end == value || *end || errno == ERANGE
              || center < std::numeric_limits<int64_t>::min()
              || center > std::numeric_limits<int64_t>::max())
            {
              fprintf (stderr, "--dump-d requires an integer sample center.\n");
              return 1;
            }
          dump_center = center;
          dump_d = true;
        }
      else if (std::strcmp (argv[arg], "--expected-f0") == 0 && arg + 1 < argc)
        {
          char *end = nullptr;
          const char *value = argv[++arg];
          params.expected_f0 = std::strtod (value, &end);
          if (end == value || *end || !std::isfinite (params.expected_f0)
              || params.expected_f0 < params.min_freq || params.expected_f0 >= params.max_freq)
            {
              fprintf (stderr, "Expected F0 must be in [%.3f, %.0f) Hz.\n", params.min_freq, params.max_freq);
              return 1;
            }
        }
      else
        {
          fprintf (stderr, "Usage: %s [--expected-f0 HZ] [--track | --dump-d SAMPLE] < samples.txt\n"
                   "Default: frame-start sample, candidate Hz, probability (0 Hz = unvoiced).\n"
                   "--track: center sample, decoded Hz, voiced flag, local voiced probability.\n"
                   "--dump-d: lag in samples, D, D1 at the exact zero-based sample center.\n"
                   "          Uses --expected-f0 filtering if given; zero-pads boundaries.\n"
                   "--encoder-output FILE: also encode the original, unfiltered audio; write\n"
                   "          center sample, env_f0 in Hz, raw env_f0 ratio to FILE.\n"
                   "--encoder-f0 HZ: encoder nominal fundamental (default 440 Hz), independent\n"
                   "          of --expected-f0. Requires --encoder-output to run the encoder.\n", argv[0]);
          return std::strcmp (argv[arg], "--help") == 0 ? 0 : 1;
        }
    }
  if (dump_d && (track || encoder_output))
    {
      fprintf (stderr, "--dump-d cannot be combined with --track or --encoder-output.\n");
      return 1;
    }

  vector<float> samples;
  char buffer[1024];
  while (fgets (buffer, sizeof (buffer), stdin))
    samples.push_back (atof (buffer));

  // Preserve the experiment's old frame support and plotting timestamps, but
  // submit every analysis center up front to the batch API.
  vector<int64_t> centers;
  for (size_t start = 0; start + 2 * win < samples.size(); start += hop)
    centers.push_back (start + win - 1);
  PYin pyin (params);
  if (dump_d)
    {
      const auto curves = pyin.difference (samples, dump_center);
      printf ("# center_sample %lld\n# lag D D1\n", static_cast<long long> (dump_center));
      for (size_t lag = 0; lag < curves.D.size(); lag++)
        printf ("%zu %.9g %.9g\n", lag, curves.D[lag], curves.D1[lag]);
      return 0;
    }
  vector<PYin::Frame> frames;
  if (!pyin.analyze (samples, centers, frames))
    return 1;
  if (encoder_output && !write_encoder_pitch (samples, encoder_f0, encoder_output))
    return 1;
  for (size_t i = 0; i < frames.size(); i++)
    {
      if (track)
        printf ("%lld %.9g %d %.9g\n", static_cast<long long> (centers[i]),
                frames[i].freq, frames[i].voiced, frames[i].voiced_probability);
      else
        {
          const auto& observation = pyin.observations()[i];
          for (const auto& candidate : observation.candidates)
            printf ("%lld %.9g %.9g\n", static_cast<long long> (centers[i] - win + 1),
                    candidate.freq, candidate.probability);
          printf ("%lld 0 %.9g\n", static_cast<long long> (centers[i] - win + 1),
                  observation.unvoiced_probability);
        }
    }
  return 0;
}
