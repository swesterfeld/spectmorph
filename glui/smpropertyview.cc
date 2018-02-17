// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpropertyview.hh"
#include "smlabel.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

PropertyView::PropertyView (Property& property) :
  property (property)
{
}

void
PropertyView::set_enabled (bool enabled)
{
  title->set_enabled (enabled);
  slider->set_enabled (enabled);
  label->set_enabled (enabled);
}

int
PropertyView::init_ui (Widget *parent, FixedGrid& grid, int yoffset)
{
  slider = new Slider (parent, 0);

  slider->set_int_range (property.min(), property.max());
  label = new Label (parent, property.value_label());
  title = new Label (parent, property.label());
  slider->set_int_value (property.get());

  connect (slider->signal_int_value_changed, this, &PropertyView::on_value_changed);

  grid.add_widget (title, 0, yoffset, 9, 2);
  grid.add_widget (slider,  9, yoffset, 25, 2);
  grid.add_widget (label, 35, yoffset, 5, 2);

  return 2;
}

void
PropertyView::on_value_changed (int value)
{
  property.set (value);
  label->text = property.value_label();
}
