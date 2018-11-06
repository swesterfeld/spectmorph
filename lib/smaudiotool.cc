// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaudiotool.hh"
#include "smnoisebandpartition.hh"

using namespace SpectMorph;

AudioTool::Block2Energy::Block2Energy (double mix_freq)
{
  NoiseBandPartition partition (32, 4096, mix_freq);

  for (size_t i = 0; i < 32; i++)
    noise_factors.push_back (mix_freq * partition.bins_per_band (i) / 4096.);
}

double
AudioTool::Block2Energy::energy (const AudioBlock& block)
{
  g_return_val_if_fail (block.noise.size() == noise_factors.size(), 0);

  double e = 0;
  for (size_t i = 0; i < block.mags.size(); i++)
    e += 0.5 * block.mags_f (i) * block.mags_f (i);

  for (size_t i = 0; i < block.noise.size(); i++)
    e += block.noise_f (i) * block.noise_f (i) * noise_factors[i];

  return e;
}
