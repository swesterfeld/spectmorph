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
  MorphPlan   *morph_plan = nullptr;
  Label       *volume_value_label = nullptr;
  Slider      *volume_slider = nullptr;
  Led         *midi_led = nullptr;
  Label       *inst_status = nullptr;
  double       m_view_height;

  void update_volume_label (double volume);
public:
  MorphPlanControl (Widget *parent, MorphPlan *plan);

  double view_height();

  void set_volume (double volume);

/* slots */
  void on_volume_changed (double new_volume);
  void on_project_volume_changed (double new_volume);
  void on_index_changed();
  void on_update_led();
};

}

#endif
