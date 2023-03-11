// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smcombobox.hh"
#include "smcheckbox.hh"
#include "smfixedgrid.hh"
#include "smcontrolview.hh"

namespace SpectMorph
{

class OperatorLayout;
class MorphPlanWindow;
class ControlStatus;

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

class PropertyViewModLabel : public Widget
{
  bool            m_pressed = false;
  std::string     m_text;
  bool            m_bold = false;
  Color           m_color = ThemeColor::TEXT;
  ModulationList *m_mod_list;
public:
  PropertyViewModLabel (Widget *parent, const std::string& text, ModulationList *mod_list) :
    Widget (parent),
    m_text (text),
    m_mod_list (mod_list)
  {
    connect (mod_list->signal_modulation_changed, this, &PropertyViewModLabel::on_update_active);
    on_update_active();
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    Color text_color = m_color;

    if (!enabled())
      text_color = text_color.darker();

    du.set_color (m_color);
    double space = 6;
    double yoffset = height() / 2 - 8;
    cairo_move_to (cr, 2, yoffset + space);
    cairo_line_to (cr, 2, yoffset + 16 - space);
    cairo_line_to (cr, 2 + (16 - 2 * space) * 0.8, yoffset + 16 / 2);

    cairo_close_path (cr);
    cairo_stroke_preserve (cr);
    cairo_fill (cr);
    du.set_color (text_color);
    du.bold = m_bold;
    du.text (m_text, 10, 0, width(), height(), TextAlign::LEFT);
  }
  void
  enter_event() override
  {
    m_bold = true;
    update();
  }
  void
  leave_event() override
  {
    m_bold = false;
    update();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON)
      {
        m_pressed = true;
        update();
      }
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button != LEFT_BUTTON || !m_pressed)
      return;

    m_pressed = false;
    update();

    if (event.x >= 0 && event.y >= 0 && event.x < width() && event.y < height())
      signal_clicked();
  }
  void
  on_update_active()
  {
    if (m_mod_list->main_control_type() != MorphOperator::CONTROL_GUI || m_mod_list->count() != 0)
      {
        m_color = ThemeColor::SLIDER;
        m_color = m_color.lighter (180);
      }
    else
      {
        m_color = ThemeColor::TEXT;
      }
    update();
  }
  Signal<> signal_clicked;
};

class PropertyView : public SignalReceiver
{
  Property& m_property;

  Widget   *title  = nullptr;
  Slider   *slider = nullptr;
  ComboBox *combobox = nullptr;
  CheckBox *check_box = nullptr;
  PropertyViewLabel *label  = nullptr;

  ControlView control_view;
  ComboBoxOperator *control_combobox = nullptr;
  Widget *control_combobox_title = nullptr;
  ControlStatus *control_status = nullptr;
  bool show_control_status = true;

  ModulationList *mod_list = nullptr;

  MorphPlanWindow *window = nullptr;

  void on_value_changed (int new_value);

public:
  PropertyView (Property& property, Widget *parent, MorphPlanWindow *window, OperatorLayout& layout);
  PropertyView (Property& property);

  ComboBox *create_combobox (Widget *parent);
  Property *property() const;

  void set_enabled (bool enabled);
  void set_visible (bool visible);
  void set_show_control_status (bool show_control_view);

/* signals: */
  Signal<> signal_visibility_changed;

/* slots: */
  void on_update_value();
  void on_edit_details();
};

}

#endif
