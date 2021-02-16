// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_HH
#define SPECTMORPH_PROPERTY_HH

#include "smmath.hh"
#include "smutils.hh"

#include <functional>
#include <string>

namespace SpectMorph
{

class EnumInfo;

class Property
{
public:
  enum class Type { INT, ENUM, FLOAT };

  virtual Type        type() = 0;

  virtual int         min() = 0;
  virtual int         max() = 0;
  virtual int         get() = 0;
  virtual void        set (int v) = 0;

  virtual std::string label() = 0;
  virtual std::string value_label() = 0;

  virtual void save (OutFile& out_file) = 0;
  virtual bool load (InFile& in_file) = 0;

  Signal<> signal_value_changed;

  /* specific types */
  virtual float get_float() const { return 0; }
  virtual void  set_float (float f) {}

  virtual const EnumInfo *enum_info() const { return nullptr; }
};

class IntProperty : public Property
{
  int          *m_value;
  std::string   m_identifier;
  int           m_min_value;
  int           m_max_value;
  std::string   m_label;
  std::string   m_format;
public:
  Type type()       { return Type::INT; }
  int min()         { return m_min_value; }
  int max()         { return m_max_value; }
  int get()         { return *m_value; }

  IntProperty (int *value, const std::string& identifier, const std::string& label, const std::string& format,
               int def, int mn, int mx) :
    m_value (value),
    m_identifier (identifier),
    m_min_value (mn),
    m_max_value (mx),
    m_label (label),
    m_format (format)
  {
    *value = def;
  }
  std::string label() { return m_label; }

  std::string
  value_label()
  {
    return string_locale_printf (m_format.c_str(), *m_value);
  }

  void
  set (int v)
  {
    *m_value = v;
    signal_value_changed();
  }
  void
  save (OutFile& out_file)
  {
    out_file.write_int (m_identifier, *m_value);
  }
  bool
  load (InFile& in_file)
  {
    if (in_file.event() == InFile::INT)
      {
        if (in_file.event_name() == m_identifier)
          {
            *m_value = in_file.event_int();
            return true;
          }
      }
    return false;
  }
};

class EnumInfo
{
public:
  struct Item
  {
    int         value;
    std::string text;
  };
  EnumInfo (const std::vector<Item>& items) :
    m_items (items)
  {
  }

  const std::vector<Item>
  items() const
  {
    return m_items;
  }
private:
  std::vector<Item> m_items;
};

class EnumProperty : public Property
{
  std::string               m_identifier;
  std::string               m_label;
  EnumInfo                  m_enum_info;
  std::function<int()>      m_read_func;
  std::function<void(int)>  m_write_func;
  int                       m_min_value;
  int                       m_max_value;
public:
  EnumProperty (const std::string& identifier,
                const std::string& label,
                int def,
                const EnumInfo& ei,
                std::function<int()> read_func,
                std::function<void(int)> write_func) :
    m_identifier (identifier),
    m_label (label),
    m_enum_info (ei),
    m_read_func (read_func),
    m_write_func (write_func)
  {
    m_write_func (def);

    g_return_if_fail (ei.items().size());
    m_min_value = ei.items()[0].value;
    m_max_value = ei.items()[0].value;
    for (auto item : ei.items())
      {
        m_min_value = std::min (item.value, m_min_value);
        m_max_value = std::max (item.value, m_max_value);
      }
  }
  Type type() { return Type::ENUM; }
  int min() { return m_min_value; }
  int max() { return m_max_value; }
  int get() { return m_read_func(); }
  void set (int v)
  {
    signal_value_changed();
    return m_write_func (v);
  }
  std::string label() { return m_label; }
  std::string value_label() { return "-"; }
  virtual const EnumInfo *enum_info() const { return &m_enum_info; }
  void
  save (OutFile& out_file)
  {
    out_file.write_int (m_identifier, m_read_func());
  }
  bool
  load (InFile& in_file)
  {
    if (in_file.event() == InFile::INT)
      {
        if (in_file.event_name() == m_identifier)
          {
            m_write_func (in_file.event_int());
            return true;
          }
      }
    return false;
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
  Type type() { return Type::FLOAT; }

  int min()         { return 0; }
  int max()         { return 1000; }
  int get()         { return lrint (value2ui (*m_value) * 1000); }

  void
  set (int v)
  {
    *m_value = ui2value (v / 1000.);
    signal_value_changed();
  }

  float
  get_float() const
  {
    return *m_value;
  }
  void
  set_float (float f)
  {
    *m_value = f;
    signal_value_changed();
  }

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

class XParamProperty : public PropertyBase
{
  double        m_min_value;
  double        m_max_value;
  double        m_slope;
public:
  XParamProperty (float *value,
                  const std::string& identifier,
                  const std::string& label,
                  const std::string& format,
                  float def_value,
                  float min_value,
                  float max_value,
                  double slope) :
    PropertyBase (value, identifier, label, format),
    m_min_value (min_value),
    m_max_value (max_value),
    m_slope (slope)
  {
    *value = def_value;
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

}

#endif
