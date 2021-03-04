// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpropertyview.hh"
#include "smlabel.hh"
#include "smoperatorlayout.hh"
#include "smcombobox.hh"
#include "smpropertyviewedit.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

PropertyView::PropertyView (MorphOperator *op, Property& property) :
  m_op (op),
  m_property (property)
{
  /* minimal constructor, to support custom layouts by creating widgets manually */
}

PropertyView::PropertyView (MorphOperator *op, Property& property, Widget *parent, OperatorLayout& op_layout) :
  m_op (op),
  m_property (property),
  window (parent->window())
{
  if (property.type() == Property::Type::ENUM)
    {
      create_combobox (parent);
      title = new Label (parent, property.label());
      op_layout.add_row (3, title, combobox);
    }
  else if (property.type() == Property::Type::BOOL)
    {
      check_box = new CheckBox (parent, property.label());
      check_box->set_checked (property.get_bool());
      op_layout.add_row (2, check_box);

      connect (check_box->signal_toggled,
        [this] (bool new_value)
          {
            m_property.set_bool (new_value);
          });
    }
  else
    {
      slider = new Slider (parent, 0);

      slider->set_int_range (property.min(), property.max());
      label = new PropertyViewLabel (parent, property.value_label());
      title = new Label (parent, property.label());
      slider->set_int_value (property.get());

      connect (label->signal_clicked, this, &PropertyView::on_edit_details);
      connect (slider->signal_int_value_changed, this, &PropertyView::on_value_changed);
      connect (property.signal_value_changed, this, &PropertyView::on_update_value);

      op_layout.add_row (2, title, slider, label);

      mod_list = property.modulation_list();
      if (mod_list)
        {
          control_combobox = control_view.create_combobox (parent,
            op,
            mod_list->main_control_type(),
            mod_list->main_control_op());

          connect (control_view.signal_control_changed,
            [this]()
               {
                 mod_list->set_main_control_type_and_op (control_view.control_type(), control_view.op());
               });
          connect (mod_list->signal_main_control_changed, [this]()
            {
              control_view.update_control_type_and_op (mod_list->main_control_type(), mod_list->main_control_op());
              signal_visibility_changed();
            });

          control_combobox_title = new Label (parent, property.label());
          op_layout.add_row (3, control_combobox_title, control_combobox);
        }
    }
}

ComboBox *
PropertyView::create_combobox (Widget *parent)
{
  int initial_value = m_property.get();

  combobox = new ComboBox (parent);
  const EnumInfo *enum_info = m_property.enum_info();
  for (auto item : enum_info->items())
    {
      combobox->add_item (item.text);

      if (initial_value == item.value)
        combobox->set_text (item.text);
    }
  connect (combobox->signal_item_changed, [this]()
    {
      const EnumInfo *enum_info = m_property.enum_info();
      std::string text = combobox->text();
      for (auto item : enum_info->items())
        if (item.text == text)
          m_property.set (item.value);
    });
  return combobox;
}

Property *
PropertyView::property() const
{
  return &m_property;
}

void
PropertyView::set_enabled (bool enabled)
{
  if (title)
    title->set_enabled (enabled);
  if (slider)
    slider->set_enabled (enabled);
  if (combobox)
    combobox->set_enabled (enabled);
  if (check_box)
    check_box->set_enabled (enabled);
  if (label)
    label->set_enabled (enabled);
  if (control_combobox)
    control_combobox->set_enabled (enabled);
  if (control_combobox_title)
    control_combobox_title->set_enabled (enabled);
}

void
PropertyView::set_visible (bool visible)
{
  const bool show_control_combobox = mod_list && mod_list->main_control_type() != MorphOperator::CONTROL_GUI;

  bool value_visible = visible && !show_control_combobox;
  bool control_visible = visible && show_control_combobox;

  if (title)
    title->set_visible (value_visible);
  if (slider)
    slider->set_visible (value_visible);
  if (combobox)
    combobox->set_visible (value_visible);
  if (check_box)
    check_box->set_visible (value_visible);
  if (label)
    label->set_visible (value_visible);

  if (control_combobox)
    control_combobox->set_visible (control_visible);
  if (control_combobox_title)
    control_combobox_title->set_visible (control_visible);
}

void
PropertyView::on_edit_details()
{
  PropertyViewEdit::create (window, m_op, m_property);
}

void
PropertyView::on_value_changed (int value)
{
  m_property.set (value);
  label->set_text (m_property.value_label());
}

void
PropertyView::on_update_value()
{
  slider->set_int_value (m_property.get());
  label->set_text (m_property.value_label());
}
