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
  std::function<std::string (float)> m_custom_formatter;

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

  std::string
  value_label()
  {
    if (m_custom_formatter)
      return m_custom_formatter (get_value (morph_op));
    else
      return string_locale_printf (m_format.c_str(), get_value (morph_op));
  }

  void
  set_custom_formatter (const std::function<std::string (float)>& formatter)
  {
    m_custom_formatter = formatter;
  }
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

class PropertyBase : public Property
{
  float        *m_value;
  std::string   m_identifier;
  std::string   m_label;
  std::string   m_format;
  std::function<std::string (float)> m_custom_formatter;
public:
  PropertyBase (float *value,
                const std::string& identifier,
                const std::string& label,
                const std::string& format) :
    m_value (value),
    m_identifier (identifier),
    m_label (label),
    m_format (format)
  {
  }
  int min()         { return 0; }
  int max()         { return 1000; }
  int get()         { return lrint (value2ui (*m_value) * 1000); }

  void
  set (int v)
  {
    *m_value = ui2value (v / 1000.);
    signal_value_changed();
  }

  Signal<> signal_value_changed;


  virtual double value2ui (double value) = 0;
  virtual double ui2value (double ui) = 0;

  std::string label() { return m_label; }

  std::string
  value_label()
  {
    if (m_custom_formatter)
      return m_custom_formatter (*m_value);
    else
      return string_locale_printf (m_format.c_str(), *m_value);
  }

  void
  set_custom_formatter (const std::function<std::string (float)>& formatter)
  {
    m_custom_formatter = formatter;
  }
  void
  save (OutFile& out_file)
  {
    out_file.write_float (m_identifier, *m_value);
  }
  bool
  load (InFile& in_file)
  {
    if (in_file.event() == InFile::FLOAT)
      {
        if (in_file.event_name() == m_identifier)
          {
            *m_value = in_file.event_float();
            return true;
          }
      }
    return false;
  }
};

class LogProperty : public PropertyBase
{
  double        m_min_value;
  double        m_max_value;
public:
  LogProperty (float *value,
               const std::string& identifier,
               const std::string& label,
               const std::string& format,
               float def_value,
               float min_value,
               float max_value) :
    PropertyBase (value, identifier, label, format),
    m_min_value (min_value),
    m_max_value (max_value)
  {
    *value = def_value;
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

class LinearProperty : public PropertyBase
{
  double m_min_value;
  double m_max_value;
public:
  LinearProperty (float *value,
                  const std::string& identifier,
                  const std::string& label,
                  const std::string& format,
                  float def_value,
                  double min_value,
                  double max_value) :
    PropertyBase (value, identifier, label, format),
    m_min_value (min_value),
    m_max_value (max_value)
  {
    *value = def_value;
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
