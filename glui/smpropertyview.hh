// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_HH
#define SPECTMORPH_PROPERTY_VIEW_HH

#include "smmorphoutput.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smcombobox.hh"
#include "smfixedgrid.hh"

namespace SpectMorph
{

class OperatorLayout;

struct PropertyView : public SignalReceiver
{
  Property& m_property;

  Label    *title  = nullptr;
  Slider   *slider = nullptr;
  ComboBox *combobox = nullptr;
  Label    *label  = nullptr;

  void on_value_changed (int new_value);

public:
  PropertyView (Property& property, Widget *parent, OperatorLayout& layout);
  PropertyView (Property& property);

  ComboBox *create_combobox (Widget *parent);
  Property *property() const;

  void set_enabled (bool enabled);
  void set_visible (bool visible);

/* slots: */
  void on_update_value();
};

}

#endif
