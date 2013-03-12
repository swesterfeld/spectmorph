// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_HH
#define SPECTMORPH_MORPH_OPERATOR_HH

#include "smoutfile.hh"
#include "sminfile.hh"

#include <map>

#include <QObject>

namespace SpectMorph
{

class MorphOperatorView;
class MorphPlan;

class MorphOperator : public QObject
{
  Q_OBJECT
protected:
  MorphPlan  *m_morph_plan;
  std::string m_name;
  std::string m_id;

  typedef std::map<std::string, MorphOperator *> OpNameMap;

  void write_operator (OutFile& file, const std::string& name, MorphOperator *op);

public:
  enum OutputType {
    OUTPUT_NONE,
    OUTPUT_AUDIO,
    OUTPUT_CONTROL
  };
  MorphOperator (MorphPlan *morph_plan);
  virtual ~MorphOperator();

  virtual const char *type() = 0;
  virtual bool save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;
  virtual void post_load (OpNameMap& op_name_map);
  virtual OutputType output_type() = 0;

  MorphPlan *morph_plan();

  std::string type_name();

  std::string name();
  void set_name (const std::string& name);

  bool can_rename (const std::string& name);

  std::string id();
  void set_id (const std::string& id);

  static MorphOperator *create (const std::string& type, MorphPlan *plan);
};

}

#endif
