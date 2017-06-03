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
  MorphOp&      morph_op;
  std::string   m_label;
  std::string   m_format;
  std::function<float(const MorphOp&)> get_value;
  std::function<void (MorphOp&, float)> set_value;
public:
  IProperty (MorphOp *morph_op,
             const std::string& label,
             const std::string& format,
             std::function<float(const MorphOp&)> get_value,
             std::function<void (MorphOp&, float)> set_value) :
    morph_op (*morph_op),
    m_label (label),
    m_format (format),
    get_value (get_value),
    set_value (set_value)
  {
  }
  int min()         { return 0; }
  int max()         { return 1000; }
  int get()         { return lrint (value2ui (get_value (morph_op)) * 1000); }
  void set (int v)  { set_value (morph_op, ui2value (v / 1000.)); }

  virtual double value2ui (double value) = 0;
  virtual double ui2value (double ui) = 0;

  std::string label() { return m_label; }
  std::string value_label() { return string_locale_printf (m_format.c_str(), get_value (morph_op)); }
};

template<class MorphOp>
class XParamProperty : public IProperty<MorphOp>
{
  double m_min_value;
  double m_max_value;
  double m_slope;
public:
  XParamProperty (MorphOp *morph_op,
                  const std::string& label,
                  const std::string& format,
                  double min_value,
                  double max_value,
                  double slope,
                  std::function<float(const MorphOp&)> get_value,
                  std::function<void (MorphOp&, float)> set_value) :
    IProperty<MorphOp> (morph_op, label, format, get_value, set_value),
    m_min_value (min_value),
    m_max_value (max_value),
    m_slope (slope)
  {
  }
  double
  value2ui (double v)
  {
    return sm_xparam_inv ((v - m_min_value) / (m_max_value - m_min_value), m_slope);
  }
  double
  ui2value (double ui)
  {
    return sm_xparam (ui, m_slope) * (m_max_value - m_min_value) + m_min_value;
  }
};

template<class MorphOp>
class LogParamProperty : public IProperty<MorphOp>
{
  double m_min_value;
  double m_max_value;
public:
  LogParamProperty (MorphOp *morph_op,
                    const std::string& label,
                    const std::string& format,
                    double min_value,
                    double max_value,
                    std::function<float(const MorphOp&)> get_value,
                    std::function<void (MorphOp&, float)> set_value) :
    IProperty<MorphOp> (morph_op, label, format, get_value, set_value),
    m_min_value (min_value),
    m_max_value (max_value)
  {
  }
  double
  value2ui (double v)
  {
    return (log (v) - log (m_min_value)) / (log (m_max_value) - log (m_min_value));
  }
  double
  ui2value (double ui)
  {
    return exp (ui * (log (m_max_value) - log (m_min_value)) + log (m_min_value));
  }
};

class MorphOutput;

struct MorphOutputProperties
{
  MorphOutputProperties (MorphOutput *output);

  XParamProperty<MorphOutput>   portamento_glide;

  XParamProperty<MorphOutput>   vibrato_depth;
  LogParamProperty<MorphOutput> vibrato_frequency;
  XParamProperty<MorphOutput>   vibrato_attack;
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

  bool                         m_vibrato;
  float                        m_vibrato_depth;
  float                        m_vibrato_frequency;
  float                        m_vibrato_attack;

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

  void           set_vibrato (bool ev);
  bool           vibrato() const;

  void           set_vibrato_depth (float depth);
  float          vibrato_depth() const;

  void           set_vibrato_frequency (float frequency);
  float          vibrato_frequency() const;

  void           set_vibrato_attack (float attack);
  float          vibrato_attack() const;

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
