// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphenvelopeview.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphEnvelopeView::MorphEnvelopeView (Widget *parent, MorphEnvelope *envelope, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, envelope, morph_plan_window),
  morph_envelope (envelope)
{
  curve_widget = new MorphCurveWidget (body_widget, envelope, morph_envelope->curve(), /* can_loop */ true);
  connect (morph_plan_window->signal_voice_status_changed, curve_widget, &MorphCurveWidget::on_voice_status_changed);
  connect (curve_widget->signal_curve_changed, [this] () { morph_envelope->set_curve (curve_widget->curve()); });
  op_layout.add_fixed (39, 24, curve_widget);
  op_layout.activate();
}

double
MorphEnvelopeView::view_height()
{
  return op_layout.height() + 5;
}
