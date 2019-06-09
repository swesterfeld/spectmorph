// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_HH
#define SPECTMORPH_MORPH_PLAN_HH

#include "smindex.hh"
#include "smmorphoperator.hh"
#include "smobject.hh"
#include "smaudio.hh"
#include "smutils.hh"
#include "smsignal.hh"

namespace SpectMorph
{

class Project;

class MorphPlan : public Object
{
public:
  class ExtraParameters
  {
  public:
    virtual std::string   section() = 0;
    virtual void          save (OutFile& out_file) = 0;
    virtual void          handle_event (InFile& ifile) = 0;
  };

protected:
  Project                     *m_project = nullptr;
  Index                        m_index;
  std::vector<MorphOperator *> m_operators;

  bool                         in_restore;

  void  clear();
  bool  load_index();
  Error load_internal (GenericIn *in, ExtraParameters *params = nullptr);

public:
  MorphPlan (Project& project);
  ~MorphPlan();

  const Index *index();
  Project     *project();

  enum AddPos {
    ADD_POS_AUTO,
    ADD_POS_END
  };

  void add_operator (MorphOperator *op, AddPos = ADD_POS_END, const std::string& name = "", const std::string& id = "", bool load_folded = false);
  const std::vector<MorphOperator *>& operators();
  void remove (MorphOperator *op);
  void move (MorphOperator *op, MorphOperator *op_next);

  void set_plan_str (const std::string& plan_str);
  void emit_plan_changed();
  void emit_index_changed();

  Error save (GenericOut *file, ExtraParameters *params = nullptr) const;
  Error load (GenericIn *in, ExtraParameters *params = nullptr);

  void load_default();

  MorphPlan *clone() const; // create a deep copy

  static std::string id_chars();
  static std::string generate_id();

  Signal<>                signal_plan_changed;
  Signal<>                signal_index_changed;
  Signal<>                signal_need_view_rebuild;
  Signal<MorphOperator *> signal_operator_removed;
  Signal<MorphOperator *> signal_operator_added;
};

typedef RefPtr<MorphPlan> MorphPlanPtr;

}

#endif
