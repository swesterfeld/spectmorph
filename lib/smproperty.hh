// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_HH
#define SPECTMORPH_PROPERTY_HH

#include "smmath.hh"
#include "smutils.hh"

#include <functional>
#include <string>

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

template<class MorphOp>
class LinearParamProperty : public IProperty<MorphOp>
{
  double m_min_value;
  double m_max_value;
public:
  LinearParamProperty (MorphOp *morph_op,
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
    return (v - m_min_value) / (m_max_value - m_min_value);
  }
  double
  ui2value (double ui)
  {
    return ui * (m_max_value - m_min_value) + m_min_value;
  }
};

}

#endif
