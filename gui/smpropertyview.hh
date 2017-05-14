// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"

#include <QLabel>
#include <QSlider>
#include <QGridLayout>

namespace SpectMorph
{

struct PropertyView : public QObject
{
  Q_OBJECT

  Property& property;

  QLabel    *title;
  QSlider   *slider;
  QLabel    *label;

public:
  PropertyView (Property& property);
  void init_ui (QGridLayout *layout, int row);
  void set_visible (bool visible);

public slots:
  void on_value_changed (int new_value);
};

}

#endif
