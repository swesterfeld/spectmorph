// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_HH
#define SPECTMORPH_MORPH_OPERATOR_HH

#include "smoutfile.hh"
#include "sminfile.hh"
#include "smsignal.hh"

#include <map>

namespace SpectMorph
{

class MorphOperatorView;
class MorphPlan;

class MorphOperator : public SignalReceiver
{
protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;
  std::string m_id;
  bool        m_folded;

  typedef std::map<std::string, MorphOperator *> OpNameMap;

  void write_operator (OutFile& file, const std::string& name, MorphOperator *op);

public:
  enum OutputType {
    OUTPUT_NONE,
    OUTPUT_AUDIO,
    OUTPUT_CONTROL
  };
  enum ControlType {
    CONTROL_GUI      = 1,
    CONTROL_SIGNAL_1 = 2,
    CONTROL_SIGNAL_2 = 3,
    CONTROL_OP       = 4
  };
  MorphOperator (MorphPlan *morph_plan);
  virtual ~MorphOperator();

  virtual const char *type() = 0;
  virtual int  insert_order() = 0;
  virtual bool save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;
  virtual void post_load (OpNameMap& op_name_map);
  virtual OutputType output_type() = 0;
  virtual std::vector<MorphOperator *> dependencies();

  MorphPlan *morph_plan();

  std::string type_name();

  std::string name();
  void set_name (const std::string& name);

  bool can_rename (const std::string& name);

  std::string id();
  void set_id (const std::string& id);

  bool folded() const;
  void set_folded (bool folded);

  static MorphOperator *create (const std::string& type, MorphPlan *plan);
};

}

#endif
