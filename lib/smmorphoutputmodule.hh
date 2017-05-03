// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_MODULE_HH
#define SPECTMORPH_MORPH_OUTPUT_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphplanvoice.hh"
#include "smlivedecoder.hh"

namespace SpectMorph {

class MorphOutputModule : public MorphOperatorModule
{
  std::vector<MorphOperatorModule *> out_ops;
  std::vector<LiveDecoder *>         out_decoders;

public:
  MorphOutputModule (MorphPlanVoice *voice);
  ~MorphOutputModule();

  void set_config (MorphOperator *op);
  void process (size_t n_samples, float **values, size_t n_ports, const float *freq_in = nullptr);
  void retrigger (int channel, float freq, int midi_velocity);
};

}

#endif
