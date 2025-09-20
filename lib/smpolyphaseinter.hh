// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <sys/types.h>
#include "smmain.hh"

namespace SpectMorph
{

class PolyPhaseInter
{
  std::vector<float> x;

public:
  static PolyPhaseInter *
  the()
  {
    static Singleton<PolyPhaseInter> singleton;
    return singleton.ptr();
  }

  PolyPhaseInter();

  double get_sample (const std::vector<float>& signal, double pos);
  double get_sample_no_check (const float *signal, double pos);

  size_t get_min_padding();
};

};
