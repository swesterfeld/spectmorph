// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smrandom.hh"
#include "smaudio.hh"
#include "smrtmemory.hh"

#include <vector>

namespace SpectMorph
{

class FormantCorrection
{
public:
  enum Mode {
    MODE_REPITCH = 1,
    MODE_PRESERVE_SPECTRAL_ENVELOPE = 2,
    MODE_HARMONIC_RESYNTHESIS = 3
  };
private:
  static constexpr int RESYNTH_MAX_PARTIALS = 1000;
  static constexpr double RESYNTH_MIN_FREQ = 6;
  static constexpr double RESYNTH_MAX_FREQ = 10;

  double             ratio = 0;
  int                max_partials = 0;
  Mode               mode = MODE_REPITCH;
  float              fuzzy_resynth = 0;
  double             fuzzy_resynth_freq = 0;
  double             fuzzy_frac = 0;
  std::vector<float> detune_factors;
  std::vector<float> next_detune_factors;
  Random             detune_random;

  void gen_detune_factors (std::vector<float>& factors, size_t partials);
public:
  FormantCorrection();

  void set_mode (Mode new_mode);
  void set_fuzzy_resynth (float new_fuzzy_resynth);
  void set_max_partials (int new_max_partials);
  void set_ratio (double ratio);

  void advance (double time_ms);
  void retrigger();
  void process_block (const AudioBlock& in_block, RTAudioBlock& out_block);
};

}
