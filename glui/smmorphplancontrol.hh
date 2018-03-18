// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_CONTROL_HH
#define SPECTMORPH_MORPH_PLAN_CONTROL_HH

#include "smwindow.hh"
#include "smmorphplan.hh"
#include "smframe.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smled.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanControl : public Frame
{
  MorphPlanPtr morph_plan;
  Label       *volume_value_label = nullptr;
  Slider      *volume_slider = nullptr;
  Led         *midi_led = nullptr;
  Label       *inst_status = nullptr;
  double       m_view_height;

public:
  enum Features {
    ALL_WIDGETS,
    NO_VOLUME
  };

  MorphPlanControl (Widget *parent, MorphPlanPtr plan, Features f = ALL_WIDGETS);

  double view_height();

  void set_volume (double volume);
  void set_led (bool on);

  Signal<double> signal_volume_changed;

/* slots */
  void on_volume_changed (double new_volume);
  void on_index_changed();
};

}

#endif
