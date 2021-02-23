// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_MODULE_HH
#define SPECTMORPH_MORPH_OUTPUT_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphplanvoice.hh"
#include "smeffectdecoder.hh"

namespace SpectMorph {

class MorphOutputModule : public MorphOperatorModule
{
  const MorphOutput::Config         *cfg;
  std::vector<MorphOperatorModule *> out_ops;
  std::vector<EffectDecoder *>       out_decoders;
  TimeInfo                           block_time;

public:
  MorphOutputModule (MorphPlanVoice *voice);
  ~MorphOutputModule();

  void set_config (const MorphOperatorConfig *op_cfg);
  void process (const TimeInfo& time_info, size_t n_samples, float **values, size_t n_ports, const float *freq_in = nullptr);
  void retrigger (const TimeInfo& time_info, int channel, float freq, int midi_velocity);
  void release();
  bool done();

  bool  portamento() const;
  float portamento_glide() const;
  float velocity_sensitivity() const;
  float filter_cutoff_mod() const;
  TimeInfo compute_time_info() const;
};

}

#endif
