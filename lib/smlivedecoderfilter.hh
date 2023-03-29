// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smladdervcf.hh"
#include "smskfilter.hh"
#include "smmorphoutput.hh"
#include "smlinearsmooth.hh"
#include "smfilterenvelope.hh"

namespace SpectMorph
{

class MorphOutputModule;

class LiveDecoderFilter
{
  LinearSmooth              cutoff_smooth;
  LinearSmooth              resonance_smooth;
  LinearSmooth              drive_smooth;
  bool                      smooth_first = false;
  float                     current_note = 60;
  float                     key_tracking = 0;
  FilterEnvelope            envelope;
  float                     depth_octaves = 0;
  float                     mix_freq = 0;

  MorphOutput::FilterType   filter_type;
  MorphOutputModule        *output_module = nullptr;

  static constexpr int FILTER_OVERSAMPLE = 4;

  LadderVCF                 ladder_filter { FILTER_OVERSAMPLE };
  SKFilter                  sk_filter { FILTER_OVERSAMPLE };

public:
  void retrigger (float note);
  void release();
  void process (size_t n_values, float *audio);

  void set_config (MorphOutputModule *output_module, const MorphOutput::Config *cfg, float mix_freq);

  int idelay();
};

}
