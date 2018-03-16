// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_CONTROL_HH
#define SPECTMORPH_MORPH_PLAN_CONTROL_HH

#include "smwindow.hh"
#include "smmorphplan.hh"
#include "smframe.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include <functional>

namespace SpectMorph
{

struct MorphPlanControl : public Frame
{
  Label       *volume_value_label;
  Slider      *volume_slider;

public:
  enum Features {
    ALL_WIDGETS,
    NO_VOLUME
  };

  MorphPlanControl (Widget *parent, MorphPlanPtr plan, Features f = ALL_WIDGETS);
  void set_volume (double volume);

  Signal<double> signal_volume_changed;

/* slots */
  void on_volume_changed (double new_volume);
};

}

#endif
