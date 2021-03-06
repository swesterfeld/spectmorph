// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_HH
#define SPECTMORPH_MORPH_OPERATOR_HH

#include "smoutfile.hh"
#include "sminfile.hh"
#include "smsignal.hh"
#include "smproperty.hh"

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

class MorphOperator : public SignalReceiver
{
public:
  typedef std::map<std::string, MorphOperator *> OpNameMap;

protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;
  std::string m_id;
  bool        m_folded;
  std::map<std::string, std::unique_ptr<Property>> m_properties;

  LogProperty *add_property_log (float *value, const std::string& identifier,
                                 const std::string& label, const std::string& value_label,
                                 float def, float mn, float mx);
  XParamProperty *add_property_xparam (float *value, const std::string& identifier,
                                 const std::string& label, const std::string& value_label,
                                 float def, float mn, float mx, float slope);
  LinearProperty *add_property (float *value, const std::string& identifier,
                                const std::string& label, const std::string& value_label,
                                float def, float mn, float mx);
  IntProperty *add_property (int *value, const std::string& identifier,
                             const std::string& label, const std::string& value_label,
                             int def, int mn, int mx);
  BoolProperty *add_property (bool *value, const std::string& identifier,
                              const std::string& label,
                              bool def);
  template<typename Enum>
  EnumProperty *
  add_property_enum (Enum *value, const std::string& identifier,
                                  const std::string& label, int def, const EnumInfo& ei)
  {
    return add_property_enum (identifier, label, def, ei,
      [value]() { return *value; },
      [value](int new_value) { *value = static_cast<Enum> (new_value); });
  }

  EnumProperty *
  add_property_enum (const std::string& identifier,
                     const std::string& label, int def, const EnumInfo& ei,
                     std::function<int()> read_func,
                     std::function<void(int)> write_func);

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

  Property *property (const std::string& identifier);
  void write_properties (OutFile& out_file);
  bool read_property_event (InFile& in_file);
  void read_properties_post_load (OpNameMap& op_name_map);

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
