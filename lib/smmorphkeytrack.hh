// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperator.hh"
#include "smcurve.hh"

namespace SpectMorph
{

class MorphKeyTrack : public MorphOperator
{
  LeakDebugger leak_debugger { "SpectMorph::MorphKeyTrack" };

public:
  struct Config : public MorphOperatorConfig
  {
    Curve curve;
  };
protected:
  Config m_config;
public:
  MorphKeyTrack (MorphPlan *morph_plan);

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
