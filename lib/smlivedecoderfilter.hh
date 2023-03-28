// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smladdervcf.hh"
#include "smskfilter.hh"
#include "smmorphoutput.hh"

namespace SpectMorph
{

class LiveDecoderFilter
{
  MorphOutput::FilterType   filter_type;

  static constexpr int FILTER_OVERSAMPLE = 4;

  LadderVCF                 ladder_filter { FILTER_OVERSAMPLE };
  SKFilter                  sk_filter { FILTER_OVERSAMPLE };

public:
  void reset();
  void process (size_t n_values, float *audio);
  void set_config (const MorphOutput::Config *cfg, float mix_freq);
};

}
