// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
void auto_tune_smooth (Audio& audio, int partials, double smooth_ms, double smooth_percent);

class Block2Energy
{
  std::vector<float> noise_factors;
public:
  Block2Energy (double mix_freq);

  double energy (const AudioBlock& block);
};

}

}

#endif
