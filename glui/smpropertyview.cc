// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpropertyview.hh"
#include "smlabel.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

PropertyView::PropertyView (Property& property, Widget *parent, OperatorLayout& op_layout) :
  property (property)
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

void
PropertyView::set_enabled (bool enabled)
{
  title->set_enabled (enabled);
  slider->set_enabled (enabled);
  label->set_enabled (enabled);
}

void
PropertyView::set_visible (bool visible)
{
  title->set_visible (visible);
  slider->set_visible (visible);
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
