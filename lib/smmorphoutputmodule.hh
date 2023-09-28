// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_OUTPUT_MODULE_HH
#define SPECTMORPH_MORPH_OUTPUT_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphplanvoice.hh"
#include "smeffectdecoder.hh"
#include "smrtmemory.hh"

namespace SpectMorph {

class MorphOutputModule : public MorphOperatorModule
{
  const MorphOutput::Config         *cfg = nullptr;
  const TimeInfoGenerator           *time_info_gen = nullptr;
  RTMemoryArea                      *m_rt_memory_area = nullptr;
  EffectDecoder                      decoder;

public:
  MorphOutputModule (MorphPlanVoice *voice);
  ~MorphOutputModule();

  void set_config (const MorphOperatorConfig *op_cfg);
  void process (const TimeInfoGenerator& time_info, RTMemoryArea& rt_memory_area, size_t n_samples, float **values, size_t n_ports, const float *freq_in = nullptr);
  void retrigger (const TimeInfo& time_info, int channel, float freq, int midi_velocity);
  void release();
  bool done();

  bool  portamento() const;
  float portamento_glide() const;
  float velocity_sensitivity() const;
  int   pitch_bend_range() const;
  float filter_cutoff_mod() const;
  float filter_resonance_mod() const;
  float filter_drive_mod() const;
  TimeInfo compute_time_info() const;
  RTMemoryArea *rt_memory_area() const;
};

}

#endif
