// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperator.hh"
#include "smcurve.hh"

namespace SpectMorph
{

class MorphEnvelope : public MorphOperator
{
  LeakDebugger2 leak_debugger2 { "SpectMorph::MorphEnvelope" };

public:
  static constexpr auto P_TIME = "time";
  static constexpr auto P_UNIT = "unit";

  enum Unit {
    UNIT_SECONDS        = 1,
    UNIT_MINUTES        = 2,
    UNIT_NOTE_1_1       = 3,
    UNIT_NOTE_1_2       = 4,
    UNIT_NOTE_1_4       = 5,
    UNIT_NOTE_1_8       = 6,
    UNIT_NOTE_1_16      = 7,
    UNIT_NOTE_1_32      = 8,
    UNIT_NOTE_1_1T      = 9,
    UNIT_NOTE_1_2T      = 10,
    UNIT_NOTE_1_4T      = 11,
    UNIT_NOTE_1_8T      = 12,
    UNIT_NOTE_1_16T     = 13,
    UNIT_NOTE_1_32T     = 14,
    UNIT_NOTE_1_1D      = 15,
    UNIT_NOTE_1_2D      = 16,
    UNIT_NOTE_1_4D      = 17,
    UNIT_NOTE_1_8D      = 18,
    UNIT_NOTE_1_16D     = 19,
    UNIT_NOTE_1_32D     = 20
  };
  struct Config : public MorphOperatorConfig
  {
    Unit  unit;
    float time;
    Curve curve;
  };
protected:
  Config m_config;
public:
  MorphEnvelope (MorphPlan *morph_plan);
  ~MorphEnvelope();

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  OutputType         output_type() override;
  MorphOperatorConfig *clone_config() override;

  const Curve& curve() const;
  void         set_curve (const Curve& curve);
};

}
