// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphWavSourceModule : public MorphOperatorModule
{
public:
  MorphWavSourceModule (MorphPlanVoice *voice);
  ~MorphWavSourceModule();

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};

}

#endif
