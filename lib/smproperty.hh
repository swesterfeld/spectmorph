// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_HH
#define SPECTMORPH_PROPERTY_HH

#include "smmath.hh"
#include "smutils.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include "smsignal.hh"

#include <functional>
#include <memory>
#include <string>

namespace SpectMorph
{

class EnumInfo;
class ModulationList;
class ModulationData;

class Property : public SignalReceiver
{
protected:
  std::unique_ptr<ModulationList> m_modulation_list;
public:
  Property();
  virtual ~Property();

  enum class Type { BOOL, INT, ENUM, FLOAT };
  enum class Scale { NONE, LINEAR, LOG };

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
  Signal<> signal_modulation_changed;

  /* specific types */
  bool get_bool()         { return get() != 0; }
  void set_bool (bool b)  { set (b ? 1 : 0); }

  virtual float get_float() const { return 0; }
  virtual void  set_float (float f) {}

  virtual const EnumInfo *enum_info() const { return nullptr; }

  ModulationList *modulation_list() { return m_modulation_list.get(); }

  void set_modulation_data (ModulationData *mod_data);

  struct Range
  {
    double min_value;
    double max_value;

    Range (double min_value, double max_value) :
      min_value (min_value),
      max_value (max_value)
    {
    }
    double
    clamp (double value) const
    {
      return sm_clamp (value, min_value, max_value);
    }
  };
  virtual Range float_range() { return Range (min(), max()); }
  virtual Scale float_scale() { return Scale::NONE; }
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

class BoolProperty : public Property
{
  bool         *m_value;
  std::string   m_identifier;
  std::string   m_label;
public:
  Type type()       { return Type::BOOL; }
  int min()         { return 0; }
  int max()         { return 1; }
  int get()         { return *m_value; }

  BoolProperty (bool *value, const std::string& identifier, const std::string& label, bool def) :
    m_value (value),
    m_identifier (identifier),
    m_label (label)
  {
    *value = def;
  }
  std::string label() { return m_label; }

  std::string
  value_label()
  {
    return "";
  }

  void
  set (int v)
  {
    *m_value = v ? true : false;
    signal_value_changed();
  }
  void
  save (OutFile& out_file)
  {
    out_file.write_bool (m_identifier, *m_value);
  }
  bool
  load (InFile& in_file)
  {
    if (in_file.event() == InFile::BOOL)
      {
        if (in_file.event_name() == m_identifier)
          {
            *m_value = in_file.event_bool();
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
    m_write_func (v);
    signal_value_changed();
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

class FloatProperty : public Property
{
protected:
  float        *m_value;
  const Range   m_range;
  const Scale   m_scale;
  std::string   m_identifier;
  std::string   m_label;
  std::string   m_format;
  std::function<std::string (float)> m_custom_formatter;
public:
  FloatProperty (float *value,
                 const Range& range,
                 Scale scale,
                 const std::string& identifier,
                 const std::string& label,
                 const std::string& format) :
    m_value (value),
    m_range (range),
    m_scale (scale),
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
    *m_value = m_range.clamp (ui2value (v / 1000.));
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
    *m_value = m_range.clamp (f);
    signal_value_changed();
  }
  Range
  float_range() override
  {
    return m_range;
  }
  Scale
  float_scale()
  {
    return m_scale;
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

class LogProperty : public FloatProperty
{
public:
  LogProperty (float *value,
               const std::string& identifier,
               const std::string& label,
               const std::string& format,
               float def_value,
               float min_value,
               float max_value) :
    FloatProperty (value, { min_value, max_value }, Scale::LOG, identifier, label, format)
  {
    *value = def_value;
  }

  double
  value2ui (double v)
  {
    return (log (v) - log (m_range.min_value)) / (log (m_range.max_value) - log (m_range.min_value));
  }
  double
  ui2value (double ui)
  {
    return exp (ui * (log (m_range.max_value) - log (m_range.min_value)) + log (m_range.min_value));
  }
};

class LinearProperty : public FloatProperty
{
public:
  LinearProperty (float *value,
                  const std::string& identifier,
                  const std::string& label,
                  const std::string& format,
                  float def_value,
                  double min_value,
                  double max_value) :
    FloatProperty (value, { min_value, max_value }, Scale::LINEAR, identifier, label, format)
  {
    *value = def_value;
  }
  double
  value2ui (double v)
  {
    return (v - m_range.min_value) / (m_range.max_value - m_range.min_value);
  }
  double
  ui2value (double ui)
  {
    return ui * (m_range.max_value - m_range.min_value) + m_range.min_value;
  }
};

class XParamProperty : public FloatProperty
{
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
    FloatProperty (value, { min_value, max_value }, /* FIXME: FILTER */ Scale::NONE, identifier, label, format),
    m_slope (slope)
  {
    *value = def_value;
  }
  double
  value2ui (double v)
  {
    return sm_xparam_inv ((v - m_range.min_value) / (m_range.max_value - m_range.min_value), m_slope);
  }
  double
  ui2value (double ui)
  {
    return sm_xparam (ui, m_slope) * (m_range.max_value - m_range.min_value) + m_range.min_value;
  }
};

}

#endif
