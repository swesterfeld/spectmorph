// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphlfoview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include "smcheckbox.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphLFOView::MorphLFOView (Widget *parent, MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo)
{
  auto pv_wave_type = add_property_view (MorphLFO::P_WAVE_TYPE, op_layout);
  connect (pv_wave_type->property()->signal_value_changed, this, &MorphLFOView::update_visible);

  pv_frequency = add_property_view (MorphLFO::P_FREQUENCY, op_layout);

  // NOTE MODE / NOTE LABEL
  FixedGrid grid;
  auto pv_note = add_property_view (MorphLFO::P_NOTE);
  auto pv_note_mode = add_property_view (MorphLFO::P_NOTE_MODE);

  note_label = new Label (body_widget, "Note");
  note_widget = new Widget (body_widget);

  grid.add_widget (pv_note->create_combobox (note_widget), 0, 0, 14, 3);
  grid.add_widget (pv_note_mode->create_combobox (note_widget), 15, 0, 14, 3);

  op_layout.add_row (3, note_label, note_widget);

  // DEPTH / CENTER / START_PHASE
  add_property_view (MorphLFO::P_DEPTH, op_layout);
  add_property_view (MorphLFO::P_CENTER, op_layout);
  add_property_view (MorphLFO::P_START_PHASE, op_layout);

  // FLAG: SYNC PHASE
  Widget *flags_widget = new Widget (body_widget);
  op_layout.add_row (2, flags_widget);

  CheckBox *sync_voices_box = new CheckBox (flags_widget, "Sync Phase for all voices");
  sync_voices_box->set_checked (morph_lfo->sync_voices());

  connect (sync_voices_box->signal_toggled, [morph_lfo] (bool new_value) {
    morph_lfo->set_sync_voices (new_value);
  });

  // FLAG: BEAT SYNC
  CheckBox *beat_sync_box = new CheckBox (flags_widget, "Beat Sync");
  beat_sync_box->set_checked (morph_lfo->beat_sync());

  connect (beat_sync_box->signal_toggled, [this, morph_lfo] (bool new_value) {
    morph_lfo->set_beat_sync (new_value);
    update_visible();
  });

  grid.add_widget (sync_voices_box, 0, 0, 20, 2);
  grid.add_widget (beat_sync_box, 26, 0, 20, 2);

  // CUSTOM SHAPE
  curve_widget = new MorphCurveWidget (body_widget, morph_lfo, morph_lfo->curve(), MorphCurveWidget::Type::LFO);
  connect (morph_plan_window->signal_voice_status_changed, curve_widget, &MorphCurveWidget::on_voice_status_changed);
  connect (curve_widget->signal_curve_changed, [this] () { this->morph_lfo->set_curve (curve_widget->curve()); });
  op_layout.add_fixed (39, 24, curve_widget);

  op_layout.activate();
  update_visible();
}

double
MorphLFOView::view_height()
{
  return op_layout.height() + 5;
}

void
MorphLFOView::update_visible()
{
  pv_frequency->set_visible (!morph_lfo->beat_sync());
  note_label->set_visible (morph_lfo->beat_sync());
  note_widget->set_visible (morph_lfo->beat_sync());
  curve_widget->set_visible (morph_lfo->custom_shape());

  op_layout.activate();
  signal_size_changed();
}
