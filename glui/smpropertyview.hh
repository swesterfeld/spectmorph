// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smcombobox.hh"
#include "smcheckbox.hh"
#include "smfixedgrid.hh"

namespace SpectMorph
{

class OperatorLayout;

class PropertyViewLabel : public Label
{
  bool pressed = false;
public:
  PropertyViewLabel (Widget *parent, const std::string& text) :
    Label (parent, text)
  {
  }
  void
  enter_event() override
  {
    set_bold (true);
    update();
  }
  void
  leave_event() override
  {
    set_bold (false);
    update();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON)
      {
        pressed = true;
        update();
      }
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button != LEFT_BUTTON || !pressed)
      return;

    pressed = false;
    update();

    if (event.x >= 0 && event.y >= 0 && event.x < width() && event.y < height())
      signal_clicked();
  }
  Signal<> signal_clicked;
};

class PropertyView : public SignalReceiver
{
  MorphOperator *m_op;
  Property& m_property;

  Label    *title  = nullptr;
  Slider   *slider = nullptr;
  ComboBox *combobox = nullptr;
  CheckBox *check_box = nullptr;
  PropertyViewLabel *label  = nullptr;

  Window *window = nullptr;

  void on_value_changed (int new_value);

public:
  PropertyView (MorphOperator *op, Property& property, Widget *parent, OperatorLayout& layout);
  PropertyView (MorphOperator *op, Property& property);

  ComboBox *create_combobox (Widget *parent);
  Property *property() const;

  void set_enabled (bool enabled);
  void set_visible (bool visible);

/* slots: */
  void on_update_value();
  void on_edit_details();
};

}

#endif
