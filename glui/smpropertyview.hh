// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smfixedgrid.hh"

namespace SpectMorph
{

struct PropertyView : public SignalReceiver
{
  Property& property;

  Label    *title;
  Slider   *slider;
  Label    *label;

public:
  PropertyView (Property& property);
  int init_ui (Widget *parent, FixedGrid& layout, int yoffset);
  void set_enabled (bool enabled);

/* slots: */
  void on_value_changed (int new_value);
};

}

#endif
