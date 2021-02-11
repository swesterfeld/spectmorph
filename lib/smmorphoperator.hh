// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_HH
#define SPECTMORPH_MORPH_OPERATOR_HH

#include "smoutfile.hh"
#include "sminfile.hh"
#include "smsignal.hh"

#include <map>
#include <memory>

namespace SpectMorph
{

struct MorphOperatorConfig
{
  virtual ~MorphOperatorConfig();
};

typedef std::shared_ptr<MorphOperatorConfig> MorphOperatorConfigP;

class MorphOperatorView;
class MorphPlan;
class MorphOperatorPtr;
class Property;
class LogProperty;
class LinearProperty;

class MorphOperator : public SignalReceiver
{
protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;
  std::string m_id;
  bool        m_folded;
  std::map<int, std::unique_ptr<Property>> m_properties;

  typedef std::map<std::string, MorphOperator *> OpNameMap;

  void write_operator (OutFile& file, const std::string& name, const MorphOperatorPtr& op);
  LogProperty *add_property_log (float *value, const std::string& identifier, int id,
                                 const std::string& label, const std::string& value_label,
                                 float def, float mn, float mx);
  LinearProperty *add_property (float *value, const std::string& identifier, int id,
                                const std::string& label, const std::string& value_label,
                                float def, float mn, float mx);

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
    CONTROL_OP       = 4,

    /* note: don't reorder items here, as we need to be compatible with old files */
    CONTROL_SIGNAL_3 = 5,
    CONTROL_SIGNAL_4 = 6
  };
  typedef uintptr_t PtrID;

  MorphOperator (MorphPlan *morph_plan);
  virtual ~MorphOperator();

  virtual const char *type() = 0;
  virtual int  insert_order() = 0;
  virtual bool save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;
  virtual void post_load (OpNameMap& op_name_map);
  virtual OutputType output_type() = 0;
  virtual std::vector<MorphOperator *> dependencies();
  virtual MorphOperatorConfig *clone_config() = 0;

  Property *property (int id);
  void write_properties (OutFile& out_file);
  bool read_property_event (InFile& in_file);

  MorphPlan *morph_plan();

  std::string type_name();

  std::string name();
  void set_name (const std::string& name);

  bool can_rename (const std::string& name);

  std::string id();
  void set_id (const std::string& id);

  bool folded() const;
  void set_folded (bool folded);

  PtrID
  ptr_id() const
  {
    /* ptr_id is derived from MorphOperator*, which means that for a given
     * MorphPlan, these ids never collide, but if the plan is modified,
     * the same ptr_id can be taken by a new MorphOperator*
     */
    return PtrID (this);
  }

  static MorphOperator *create (const std::string& type, MorphPlan *plan);
};

class MorphOperatorPtr
{
private:
  MorphOperator *m_ptr = nullptr;
public:
  operator bool() const { return m_ptr != nullptr; };
  MorphOperator *get() const { return m_ptr; }

  MorphOperator::PtrID
  ptr_id() const
  {
    return MorphOperator::PtrID (m_ptr);
  }

  void
  set (MorphOperator *ptr)
  {
    m_ptr = ptr;
  };
};

}

#endif
