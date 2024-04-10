// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_AUDIO_TOOL_HH
#define SPECTMORPH_AUDIO_TOOL_HH

#include "smaudio.hh"

namespace SpectMorph
{

namespace AudioTool
{

double compute_energy (const Audio& audio);
void normalize_factor (double norm, Audio& audio);
void normalize_energy (double energy, Audio& audio);

bool get_auto_tune_factor (Audio& audio, double& tune_factor);
void apply_auto_tune_factor (Audio& audio, double tune_factor);
void apply_auto_tune_factor (AudioBlock& audio_block, double tune_factor);
void auto_tune_smooth (Audio& audio, int partials, double smooth_ms, double smooth_percent);

class Block2Energy
{
  std::vector<float> noise_factors;
public:
  Block2Energy (double mix_freq);

  double energy (const AudioBlock& block);
};

class FundamentalEst
{
  // 1 extra element to make code more readable: best_freq[1] / best_mag[1] corresponds to partial 1
  std::array<double, 4> m_best_freq {};
  std::array<double, 4> m_best_mag {};
public:
  void   add_partial (double freq, double mag);
  double fundamental (int n_partials) const;
};

}

}

#endif
