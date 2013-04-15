// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_HH
#define SPECTMORPH_MORPH_PLAN_HH

#include "smindex.hh"
#include "smmorphoperator.hh"
#include "smobject.hh"
#include "smaudio.hh"

namespace SpectMorph
{

class MorphPlan : public Object
{
  Q_OBJECT
  Index                        m_index;
  std::vector<MorphOperator *> m_operators;
  int                          m_structure_version;

  std::string                  index_filename;
  bool                         in_restore;

public:
  MorphPlan();
  ~MorphPlan();

  bool         load_index (const std::string& filename);
  const Index *index();

  enum AddPos {
    ADD_POS_AUTO,
    ADD_POS_END
  };

  void add_operator (MorphOperator *op, AddPos = ADD_POS_END, const std::string& name = "", const std::string& id = "");
  const std::vector<MorphOperator *>& operators();
  void remove (MorphOperator *op);
  void move (MorphOperator *op, MorphOperator *op_next);

  void set_plan_str (const std::string& plan_str);
  void emit_plan_changed();
  void emit_index_changed();

  BseErrorType save (GenericOut *file);
  BseErrorType load (GenericIn *in);
  void clear();

  int  structure_version();

  static std::string id_chars();
  static std::string generate_id();

signals:
  void plan_changed();
  void index_changed();
  void operator_removed (MorphOperator *op);
};

typedef RefPtr<MorphPlan> MorphPlanPtr;

}

#endif
