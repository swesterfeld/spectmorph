// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_MODULE_HH
#define SPECTMORPH_MORPH_OUTPUT_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphplanvoice.hh"
#include "smeffectdecoder.hh"

namespace SpectMorph {

class MorphOutputModule : public MorphOperatorModule
{
  std::vector<MorphOperatorModule *> out_ops;
  std::vector<EffectDecoder *>       out_decoders;

  bool  m_portamento;
  float m_portamento_glide;

public:
  MorphOutputModule (MorphPlanVoice *voice);
  ~MorphOutputModule();

  void set_config (MorphOperator *op);
  void process (size_t n_samples, float **values, size_t n_ports, const float *freq_in = nullptr);
  void retrigger (int channel, float freq, int midi_velocity);
  void release();
  bool done();

  bool  portamento() const;
  float portamento_glide() const;
};

}

#endif
