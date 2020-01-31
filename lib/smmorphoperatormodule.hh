// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_MODULE_HH
#define SPECTMORPH_MORPH_OPERATOR_MODULE_HH

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"
#include "smrandom.hh"

#include <string>

namespace SpectMorph
{

class MorphPlanVoice;

class MorphModuleSharedState
{
public:
  MorphModuleSharedState();
  virtual ~MorphModuleSharedState();
};

class MorphOperatorModule
{
protected:
  MorphPlanVoice                     *morph_plan_voice;
  std::vector<MorphOperatorModule *>  m_dependencies;
  int                                 m_update_value_tag;

  Random *random_gen() const;
  void clear_dependencies();
  void add_dependency (MorphOperatorModule *dep_mod);
public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual void set_config (MorphOperator *op) = 0;
  virtual LiveDecoderSource *source();
  virtual float value();
  virtual void reset_value();
  virtual void update_value (double time_ms);
  virtual void update_shared_state (double time_ms);

  const std::vector<MorphOperatorModule *>& dependencies() const;
  int& update_value_tag();

  static MorphOperatorModule *create (MorphOperator *op, MorphPlanVoice *voice);
};

}

#endif
