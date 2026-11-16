// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

// The API is still a prototype inside testyin.cc. Exercise that implementation
// directly until it moves to lib/smpyin.hh/.cc.
#define main testyin_cli_main
#include "testyin.cc"
#undef main

#include <cassert>

namespace
{

void
check_probabilities (const PYin& pyin)
{
  for (const auto& observation : pyin.observations())
    {
      double sum = observation.unvoiced_probability;
      double last_freq = 0;
      assert (sum >= 0 && sum <= 1);
      for (const auto& candidate : observation.candidates)
        {
          assert (candidate.freq >= last_freq);
          assert (candidate.probability >= 0 && candidate.probability <= 1);
          last_freq = candidate.freq;
          sum += candidate.probability;
        }
      assert (std::abs (sum - 1) < 1e-12);
    }
}

// Compare internal Viterbi decoding with exhaustive enumeration of every path
// through a small model, using irregular times and explicitly normalized rows.
void
check_viterbi()
{
  PYin::Params params;
  params.mix_freq = 8000;
  params.min_freq = 200;
  params.max_freq = 205;
  params.window_size = 400;
  vector<float> samples (4000);
  double phase = 0;
  for (size_t i = 0; i < samples.size(); i++)
    {
      phase += 2 * std::acos (-1.0) * (201 + 3.0 * i / samples.size()) / params.mix_freq;
      samples[i] = std::sin (phase);
    }
  vector<int64_t> centers {1200, 1201, 1204, 2700};
  PYin pyin (params);
  vector<PYin::Frame> frames;
  assert (pyin.analyze (samples, centers, frames));
  check_probabilities (pyin);
  int bins = 1 + std::floor (120 * std::log2 (params.max_freq / params.min_freq));
  int states = 2 * bins;
  auto log_prob = [] (double p) { return std::log (std::max (p, 1e-300)); };
  vector<vector<double>> emissions (centers.size(), vector<double> (states));
  vector<vector<vector<double>>> transitions (centers.size(), vector<vector<double>> (states, vector<double> (states)));
  for (size_t t = 0; t < centers.size(); t++)
    {
      const auto& observation = pyin.observations()[t];
      for (const auto& candidate : observation.candidates)
        {
          int bin = std::lround (120 * std::log2 (candidate.freq / params.min_freq));
          bin = std::max (0, std::min (bins - 1, bin));
          emissions[t][bin] += candidate.probability;
        }
      for (int state = bins; state < states; state++)
        emissions[t][state] = observation.unvoiced_probability / bins;
      if (t == 0)
        continue;
      double dt = double (centers[t] - centers[t - 1]) / params.mix_freq;
      int radius = std::min (bins - 1, int (std::floor (35.92 * dt * 120)));
      double change = (1 - std::pow (0.98, dt / 0.01)) / 2;
      for (int src = 0; src < states; src++)
        {
          double norm = 0;
          for (int dst = 0; dst < bins; dst++)
            norm += std::max (0, radius + 1 - std::abs (src % bins - dst));
          for (int dst = 0; dst < states; dst++)
            transitions[t][src][dst] = std::max (0, radius + 1 - std::abs (src % bins - dst % bins)) / norm
                                      * ((src < bins) == (dst < bins) ? 1 - change : change);
        }
    }
  double best = -std::numeric_limits<double>::infinity();
  vector<int> path (centers.size()), best_path;
  std::function<void(size_t, double)> enumerate = [&] (size_t t, double score)
    {
      if (t == centers.size())
        {
          if (score > best)
            {
              best = score;
              best_path = path;
            }
          return;
        }
      for (int state = 0; state < states; state++)
        {
          double transition = t == 0 ? 1.0 / states : transitions[t][path[t - 1]][state];
          if (transition == 0)
            continue;
          path[t] = state;
          enumerate (t + 1, score + std::log (transition) + log_prob (emissions[t][state]));
        }
    };
  enumerate (0, 0);
  assert (!best_path.empty());
  for (size_t t = 0; t < centers.size(); t++)
    {
      assert (frames[t].voiced == (best_path[t] < bins));
      double freq = frames[t].voiced ? params.min_freq * std::exp2 (best_path[t] / 120.0) : 0;
      assert (std::abs (frames[t].freq - freq) < 1e-10);
    }
}

void
check_difference()
{
  PYin::Params params;
  params.mix_freq = 16;
  params.min_freq = 2;
  params.max_freq = 4;
  params.window_size = 4;
  PYin pyin (params);
  vector<float> ramp (13);
  for (size_t i = 0; i < ramp.size(); i++)
    ramp[i] = i;
  // Full support is 13 samples; center 6 starts at the first input sample.
  auto curves = pyin.difference (ramp, 6);
  assert (curves.D.size() == 10 && curves.D1.size() == 10);
  assert (curves.D[0] == 0 && curves.D1[0] == 1);
  for (size_t lag = 1; lag < curves.D.size(); lag++)
    {
      assert (curves.D[lag] == 4 * lag * lag);
      double expected = 6.0 * lag * lag / ((lag + 1) * (2 * lag + 1));
      assert (std::abs (curves.D1[lag] - expected) < 1e-6);
    }
  auto padded = pyin.difference (ramp, -100);
  for (size_t lag = 0; lag < padded.D.size(); lag++)
    assert (padded.D[lag] == 0 && padded.D1[lag] == 1);

  // Diagnostic filtering must match filtering the continuous input first.
  params.expected_f0 = 2;
  PYin filtered (params);
  auto with_filter = filtered.difference (ramp, 6);
  assert (filter_for_pitch (ramp, params.mix_freq, params.expected_f0, {}));
  auto reference = pyin.difference (ramp, 6);
  assert (with_filter.D == reference.D && with_filter.D1 == reference.D1);
}

}

int
main()
{
  PYin::Params params;
  params.mix_freq = 16000;
  params.min_freq = 100;
  params.max_freq = 500;
  params.window_size = 600;
  PYin pyin (params);
  vector<float> samples (8000);
  for (size_t i = 0; i < samples.size(); i++)
    samples[i] = std::sin (2 * std::acos (-1.0) * 220 * i / params.mix_freq);
  vector<int64_t> centers {1000, 1100, 1500, 1900, 6000};
  vector<PYin::Frame> frames;
  assert (pyin.analyze (samples, centers, frames));
  assert (frames.size() == centers.size());
  for (const auto& frame : frames)
    {
      assert (frame.voiced);
      assert (std::abs (1200 * std::log2 (frame.freq / 220)) < 6);
    }
  check_probabilities (pyin);

  // Moving the input and all requested centers equally preserves the results.
  vector<float> shifted (1000, 0);
  shifted.insert (shifted.end(), samples.begin(), samples.end());
  vector<int64_t> shifted_centers = centers;
  for (auto& center : shifted_centers)
    center += 1000;
  vector<PYin::Frame> shifted_frames;
  assert (pyin.analyze (shifted, shifted_centers, shifted_frames));
  for (size_t i = 0; i < frames.size(); i++)
    {
      assert (frames[i].freq == shifted_frames[i].freq);
      assert (frames[i].voiced_probability == shifted_frames[i].voiced_probability);
    }

  // Cancellation during validation, observations, Viterbi and backtracking.
  for (int stop : {1, 3, 8, 13})
    {
      int calls = 0;
      assert (!pyin.analyze (samples, centers, frames, [&] { return ++calls == stop; }));
      assert (frames.empty() && pyin.observations().empty());
    }
  params.expected_f0 = 200;
  PYin filtered (params);
  int calls = 0;
  assert (!filtered.analyze (samples, centers, frames, [&] { return ++calls == 4; }));
  assert (frames.empty() && filtered.observations().empty());
  assert (filtered.analyze (samples, centers, frames));
  for (const auto& frame : frames)
    assert (frame.voiced && std::abs (frame.freq - 220) < 1);

  vector<int64_t> edges {std::numeric_limits<int64_t>::min(), -5000, 0, 500, 20000,
                         std::numeric_limits<int64_t>::max()};
  assert (pyin.analyze ({}, edges, frames));
  for (const auto& frame : frames)
    assert (!frame.voiced && frame.freq == 0 && frame.voiced_probability == 0);
  assert (pyin.analyze (samples, {}, frames));
  assert (frames.empty() && pyin.observations().empty());
  assert (pyin.analyze (vector<float> (8000, 0.5), centers, frames));
  for (const auto& frame : frames)
    assert (!frame.voiced && frame.voiced_probability == 0);

  check_viterbi();
  check_difference();
  puts ("PYin API and Viterbi checks passed");
}
