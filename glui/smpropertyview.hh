// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smfixedgrid.hh"

namespace SpectMorph
{

class OperatorLayout;

struct PropertyView : public SignalReceiver
{
  Property& property;

  Label    *title;
  Slider   *slider;
  Label    *label;

  void on_value_changed (int new_value);

public:
  PropertyView (Property& property);
  int init_ui (Widget *parent, FixedGrid& layout, int yoffset);
  void init_ui (Widget *parent, OperatorLayout& layout);
  void set_enabled (bool enabled);
  void set_visible (bool visible);

/* signals: */
  Signal<> signal_value_changed;

/* slots: */
  void on_update_value();
};

}

#endif
