// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PARAM_LABEL_HH
#define SPECTMORPH_PARAM_LABEL_HH

#include "smlineedit.hh"
#include "smlabel.hh"
#include "smwindow.hh"

namespace SpectMorph
{

class ParamLabelModel
{
public:
  ParamLabelModel();
  virtual ~ParamLabelModel();

  virtual std::string value_text() = 0;
  virtual std::string display_text() = 0;
  virtual void        set_value_text (const std::string& t) = 0;
};

class ParamLabelModelDouble : public ParamLabelModel
{
  double value;
  double min_value;
  double max_value;

  std::string value_fmt;
  std::string display_fmt;

public:
  ParamLabelModelDouble (double start, double min_val, double max_val, const std::string& value_fmt, const std::string& display_fmt) :
    value (start),
    min_value (min_val),
    max_value (max_val),
    value_fmt (value_fmt),
    display_fmt (display_fmt)
  {
  }

  std::string
  value_text()
  {
    return string_locale_printf (value_fmt.c_str(), value);
  }
  std::string
  display_text()
  {
    return string_locale_printf (display_fmt.c_str(), value);
  }
  void
  set_value_text (const std::string& t)
  {
    value = atof (t.c_str());

    value = sm_bound (min_value, value, max_value);

    signal_value_changed (value);
  }
  Signal<double> signal_value_changed;
};

class ParamLabelModelInt : public ParamLabelModel
{
  int value;
  int min_value;
  int max_value;

public:
  ParamLabelModelInt (int i, int min_value, int max_value) :
    value (i),
    min_value (min_value),
    max_value (max_value)
  {
  }

  std::string
  value_text()
  {
    return string_locale_printf ("%d", value);
  }
  std::string
  display_text()
  {
    return string_locale_printf ("%d", value);
  }
  void
  set_value_text (const std::string& t)
  {
    value = atoi (t.c_str());

    value = sm_bound (min_value, value, max_value);

    signal_value_changed (value);
  }
  Signal<int> signal_value_changed;
};

class ParamLabelModelString : public ParamLabelModel
{
  std::string value;

public:
  ParamLabelModelString (const std::string& s):
    value (s)
  {
  }
  std::string
  value_text()
  {
    return value;
  }
  std::string
  display_text()
  {
    return value;
  }
  void
  set_value_text (const std::string& t)
  {
    value = t;

    signal_value_changed (value);
  }
  Signal<std::string> signal_value_changed;
};

class ParamLabel : public Label
{
  bool      pressed = false;
  LineEdit *line_edit = nullptr;

  std::unique_ptr<ParamLabelModel> model;
public:
  ParamLabel (Widget *parent, ParamLabelModel *model) :
    Label (parent, ""),
    model (model)
  {
    set_text (model->display_text());
  }
  void
  mouse_press (double x, double y) override
  {
    pressed = true;
  }
  void
  mouse_release (double x, double y) override
  {
    if (!pressed)
      return;
    pressed = false;

    if (!line_edit)
      {
        line_edit = new LineEdit (this, model->value_text());
        line_edit->height = height;
        line_edit->width = width;
        line_edit->x = 0;
        line_edit->y = 0;

        connect (line_edit->signal_return_pressed, this, &ParamLabel::on_return_pressed);
        connect (line_edit->signal_focus_out, this, &ParamLabel::on_return_pressed);

        window()->set_keyboard_focus (line_edit, true);

        set_text ("");
      }
  }
  void
  on_return_pressed()
  {
    if (!line_edit->visible())
      return;

    model->set_value_text (line_edit->text());
    set_text (model->display_text());
    line_edit->delete_later();
    line_edit = nullptr;
  }
};

}

#endif
