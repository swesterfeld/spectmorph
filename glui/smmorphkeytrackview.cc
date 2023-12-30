// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphkeytrackview.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphKeyTrackView::MorphKeyTrackView (Widget *parent, MorphKeyTrack *key_track, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, key_track, morph_plan_window),
  morph_key_track (key_track)
{
  curve_widget = new MorphCurveWidget (body_widget, key_track, morph_key_track->curve(), MorphCurveWidget::Type::KEY_TRACK);
  connect (morph_plan_window->signal_voice_status_changed, curve_widget, &MorphCurveWidget::on_voice_status_changed);
  connect (curve_widget->signal_curve_changed, [this] () { morph_key_track->set_curve (curve_widget->curve()); });
  op_layout.add_fixed (39, 24, curve_widget);
  op_layout.activate();
}

double
MorphKeyTrackView::view_height()
{
  return op_layout.height() + 5;
}
