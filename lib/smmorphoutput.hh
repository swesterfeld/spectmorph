// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_HH
#define SPECTMORPH_MORPH_OUTPUT_HH

#include "smmorphoperator.hh"
#include "smutils.hh"
#include "smmath.hh"

#include <string>
#include <functional>

namespace SpectMorph
{

class Property
{
public:
  virtual int         min() = 0;
  virtual int         max() = 0;
  virtual int         get() = 0;
  virtual void        set (int v) = 0;

  virtual std::string label() = 0;
  virtual std::string value_label() = 0;
};

template<class MorphOp>
class IProperty : public Property
{
  MorphOp& morph_op;
  std::function<float(const MorphOp&)> get_value;
  std::function<void (MorphOp&, float)> set_value;
public:
  IProperty (MorphOp *morph_op,
             std::function<float(const MorphOp&)> get_value,
             std::function<void (MorphOp&, float)> set_value) :
    morph_op (*morph_op),
    get_value (get_value),
    set_value (set_value)
  {
  }
  int min()         { return 0; }
  int max()         { return 1000; }
  int get()         { return lrint (sm_xparam_inv (get_value (morph_op), 3) * 1000); }
  void set (int v)  { set_value (morph_op, sm_xparam (v / 1000., 3)); }

  std::string label() { return "Glide"; }
  std::string value_label() { return string_printf ("%.2f ms", get_value (morph_op) * 1000); }
};

class MorphOutput : public MorphOperator
{
  Q_OBJECT

  std::vector<std::string>     load_channel_op_names;
  std::vector<MorphOperator *> channel_ops;

  bool                         m_sines;
  bool                         m_noise;

  bool                         m_unison;
  int                          m_unison_voices;
  float                        m_unison_detune;

  bool                         m_portamento;
  float                        m_portamento_glide;
  IProperty<MorphOutput>       m_portamento_glide_property;

public:
  MorphOutput (MorphPlan *morph_plan);
  ~MorphOutput();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load (OpNameMap& op_name_map);
  OutputType         output_type();

  void           set_sines (bool es);
  bool           sines() const;

  void           set_noise (bool en);
  bool           noise() const;

  void           set_unison (bool eu);
  bool           unison() const;

  void           set_unison_voices (int voices);
  int            unison_voices() const;

  void           set_unison_detune (float voices);
  float          unison_detune() const;

  void           set_portamento (bool ep);
  bool           portamento() const;

  void           set_portamento_glide (float glide);
  float          portamento_glide() const;

  Property*      portamento_glide_property();

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
