// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_AUDIO_TOOL_HH
#define SPECTMORPH_AUDIO_TOOL_HH

#include "smaudio.hh"

namespace SpectMorph
{

namespace AudioTool
{

double compute_energy (const Audio& audio);

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
