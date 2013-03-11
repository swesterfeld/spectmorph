// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoperatormodule.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphGridModule : public MorphOperatorModule
{
public:
  MorphGridModule (MorphPlanVoice *voice);
  ~MorphGridModule();

  void set_config (MorphOperator *op);
};
}
