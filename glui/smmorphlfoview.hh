// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"
#include "smenumview.hh"
#include "smoperatorlayout.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
  MorphLFO  *morph_lfo;

  MorphLFOProperties       morph_lfo_properties;

  Label     *note_label;
  Widget    *note_widget;

  PropertyView pv_frequency;
  PropertyView pv_depth;
  PropertyView pv_center;
  PropertyView pv_start_phase;

  EnumView ev_wave_type;
  EnumView ev_note;
  EnumView ev_note_mode;

  OperatorLayout op_layout;

  void update_visible();
public:
  MorphLFOView (Widget *widget, MorphLFO *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
