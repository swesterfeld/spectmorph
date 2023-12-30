// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"
#include "smoperatorlayout.hh"
#include "smmorphcurvewidget.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
  MorphLFO         *morph_lfo = nullptr;
  MorphCurveWidget *curve_widget = nullptr;

  Label            *note_label = nullptr;
  Widget           *note_widget = nullptr;

  PropertyView     *pv_frequency = nullptr;

  OperatorLayout    op_layout;

  void update_visible() override;
public:
  MorphLFOView (Widget *widget, MorphLFO *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}
