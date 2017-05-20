// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_POLY_PHASE_INTER_HH
#define SPECTMORPH_POLY_PHASE_INTER_HH

#include <vector>

namespace SpectMorph
{

class PolyPhaseInter
{
  PolyPhaseInter();
  ~PolyPhaseInter() {}

  std::vector<float> x;

public:
  static PolyPhaseInter *the();

  double get_sample (const std::vector<float>& signal, double pos);
  double get_sample_no_check (const std::vector<float>& signal, double pos);
};

};

#endif
