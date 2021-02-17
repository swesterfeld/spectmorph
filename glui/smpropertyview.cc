// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpropertyview.hh"
#include "smlabel.hh"
#include "smoperatorlayout.hh"
#include "smcombobox.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

PropertyView::PropertyView (Property& property) :
  property (property)
{
  /* minimal constructor, to support custom layouts by creating widgets manually */
}

PropertyView::PropertyView (Property& property, Widget *parent, OperatorLayout& op_layout) :
  property (property)
{
  if (property.type() == Property::Type::ENUM)
    {
      create_combobox (parent);
      title = new Label (parent, property.label());
      op_layout.add_row (3, title, combobox);
    }
  else
    {
      slider = new Slider (parent, 0);

      slider->set_int_range (property.min(), property.max());
      label = new Label (parent, property.value_label());
      title = new Label (parent, property.label());
      slider->set_int_value (property.get());

      connect (slider->signal_int_value_changed, this, &PropertyView::on_value_changed);
      connect (property.signal_value_changed, this, &PropertyView::on_update_value);

      op_layout.add_row (2, title, slider, label);
    }
}

ComboBox *
PropertyView::create_combobox (Widget *parent)
{
  int initial_value = property.get();

  combobox = new ComboBox (parent);
  const EnumInfo *enum_info = property.enum_info();
  for (auto item : enum_info->items())
    {
      combobox->add_item (item.text);

      if (initial_value == item.value)
        combobox->set_text (item.text);
    }
  connect (combobox->signal_item_changed, [this]()
    {
      const EnumInfo *enum_info = property.enum_info();
      std::string text = combobox->text();
      for (auto item : enum_info->items())
        if (item.text == text)
          property.set (item.value);
    });
  return combobox;
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
  if (label)
    label->set_enabled (enabled);
}

void
PropertyView::set_visible (bool visible)
{
  if (title)
    title->set_visible (visible);
  if (slider)
    slider->set_visible (visible);
  if (combobox)
    combobox->set_visible (visible);
  if (label)
    label->set_visible (visible);
}

void
PropertyView::on_value_changed (int value)
{
  property.set (value);
  label->set_text (property.value_label());
}

void
PropertyView::on_update_value()
{
  slider->set_int_value (property.get());
  label->set_text (property.value_label());
}
