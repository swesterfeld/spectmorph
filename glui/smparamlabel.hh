// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PARAM_LABEL_HH
#define SPECTMORPH_PARAM_LABEL_HH

#include "smlineedit.hh"

namespace SpectMorph
{

class ParamLabelModel
{
public:
  virtual std::string value_text() = 0;
  virtual std::string display_text() = 0;
  virtual void        set_value_text (const std::string& t) = 0;
};

class ParamLabelModelDouble : public ParamLabelModel
{
public:
  double db = 0;

  std::string
  value_text()
  {
    return string_locale_printf ("%.2f", db);
  }
  std::string
  display_text()
  {
    return string_locale_printf ("%.2f dB", db);
  }
  void
  set_value_text (const std::string& t)
  {
    db = atof (t.c_str());

    signal_value_changed (db);
  }
  Signal<double> signal_value_changed;
};

class ParamLabelModelString : public ParamLabelModel
{
public:
  std::string s;

  std::string
  value_text()
  {
    return s;
  }
  std::string
  display_text()
  {
    return s;
  }
  void
  set_value_text (const std::string& t)
  {
    s = t;

    signal_value_changed (s);
  }
  Signal<std::string> signal_value_changed;
};

class ParamLabel : public Label
{
  bool      pressed = false;
  LineEdit *line_edit = nullptr;

  ParamLabelModel *model;
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
