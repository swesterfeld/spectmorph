#include "spectmorphosc.genidl.hh"

namespace SpectMorph {

using namespace Bse;

class Osc : public OscBase {
  struct Properties : public OscProperties {
    Properties (Osc *osc) : OscProperties (osc)
    {
      // TODO
    }
  };
  class Module : public SynthesisModule {
  public:
    void reset()
    {
      // TODO
    }
    void process (unsigned int samples)
    {
      // TODO
    }
    void
    config (Properties *properties)
    {
      // TODO
    }
  };
public:
  /* implement creation and config methods for synthesis Module */
  BSE_EFFECT_INTEGRATE_MODULE (Osc, Module, Properties);
};

BSE_CXX_DEFINE_EXPORTS();
BSE_CXX_REGISTER_EFFECT (Osc);

}
