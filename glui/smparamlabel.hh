// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PARAM_LABEL_HH
#define SPECTMORPH_PARAM_LABEL_HH

#include "smlineedit.hh"

namespace SpectMorph
{

class ParamLabelModel
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
  }
};

class ParamLabel : public Label
{
  bool      pressed = false;
  LineEdit *line_edit = nullptr;

  ParamLabelModel *model;
public:
  ParamLabel (Widget *parent, const std::string& text) :
    Label (parent, text)
  {
    model = new ParamLabelModel();
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
    signal_value_changed (model->db);
    set_text (model->display_text());
    line_edit->delete_later();
    line_edit = nullptr;
  }
  Signal<double> signal_value_changed;
};

}

#endif
