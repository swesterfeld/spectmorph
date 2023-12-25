// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperatorview.hh"
#include "smmorphkeytrack.hh"
#include "smoperatorlayout.hh"
#include "smmorphcurvewidget.hh"

namespace SpectMorph
{

class MorphKeyTrackView : public MorphOperatorView
{
  MorphKeyTrack    *morph_key_track;
  MorphCurveWidget *curve_widget;

  OperatorLayout op_layout;
public:
  MorphKeyTrackView (Widget *widget, MorphKeyTrack *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}
