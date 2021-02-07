// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_MODULE_HH
#define SPECTMORPH_MORPH_OPERATOR_MODULE_HH

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"
#include "smrandom.hh"

#include <string>

namespace SpectMorph
{

class TimeInfo
{
public:
  double time_ms = 0;
  double ppq_pos = 0;
};

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
  TimeInfo time_info() const;
public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual void set_config (const MorphOperatorConfig *op_cfg) = 0;
  virtual LiveDecoderSource *source();
  virtual float value();
  virtual void reset_value (const TimeInfo& time_info);
  virtual void update_shared_state (const TimeInfo& time_info);

  const std::vector<MorphOperatorModule *>& dependencies() const;
  int& update_value_tag();

  static MorphOperatorModule *create (const std::string& type, MorphPlanVoice *voice);
};

}

#endif
