// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatorview.hh"
#include "smmorphenvelope.hh"
#include "smoperatorlayout.hh"
#include "smmorphcurvewidget.hh"

namespace SpectMorph
{

class MorphEnvelopeView : public MorphOperatorView
{
  MorphEnvelope    *morph_envelope;
  MorphCurveWidget *curve_widget;

  OperatorLayout op_layout;
public:
  MorphEnvelopeView (Widget *widget, MorphEnvelope *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}
