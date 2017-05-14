// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpropertyview.hh"

#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

PropertyView::PropertyView (Property& property) :
  property (property)
{
}

void
PropertyView::set_visible (bool visible)
{
  title->setVisible (visible);
  slider->setVisible (visible);
  label->setVisible (visible);
}

void
PropertyView::init_ui (QGridLayout *grid_layout, int row)
{
  slider = new QSlider (Qt::Horizontal);
  slider->setRange (property.min(), property.max());
  label = new QLabel(property.value_label().c_str());
  title = new QLabel(property.label().c_str());
  slider->setValue (property.get());

  QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed (int)));

  grid_layout->addWidget (title, row, 0);
  grid_layout->addWidget (slider, row, 1);
  grid_layout->addWidget (label, row, 2);
}

void
PropertyView::on_value_changed (int value)
{
  property.set (value);
  label->setText (property.value_label().c_str());
}
