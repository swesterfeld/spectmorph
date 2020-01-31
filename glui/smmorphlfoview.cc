// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfoview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include "smcheckbox.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

#define WAVE_TEXT_SINE          "Sine"
#define WAVE_TEXT_TRIANGLE      "Triangle"
#define WAVE_TEXT_SAW_UP        "Saw Up"
#define WAVE_TEXT_SAW_DOWN      "Saw Down"
#define WAVE_TEXT_SQUARE        "Square"
#define WAVE_TEXT_RANDOM_SH     "Random Sample & Hold"
#define WAVE_TEXT_RANDOM_LINEAR "Random Linear"

MorphLFOView::MorphLFOView (Widget *parent, MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo),
  morph_lfo_properties (morph_lfo),
  pv_frequency (morph_lfo_properties.frequency),
  pv_depth (morph_lfo_properties.depth),
  pv_center (morph_lfo_properties.center),
  pv_start_phase (morph_lfo_properties.start_phase)
{
  OperatorLayout op_layout;

  // WAVE TYPE
  wave_type_combobox = new ComboBox (body_widget);
  wave_type_combobox->add_item (WAVE_TEXT_SINE);
  wave_type_combobox->add_item (WAVE_TEXT_TRIANGLE);
  wave_type_combobox->add_item (WAVE_TEXT_SAW_UP);
  wave_type_combobox->add_item (WAVE_TEXT_SAW_DOWN);
  wave_type_combobox->add_item (WAVE_TEXT_SQUARE);
  wave_type_combobox->add_item (WAVE_TEXT_RANDOM_SH);
  wave_type_combobox->add_item (WAVE_TEXT_RANDOM_LINEAR);

  if (morph_lfo->wave_type() == MorphLFO::WAVE_SINE)
    wave_type_combobox->set_text (WAVE_TEXT_SINE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_TRIANGLE)
    wave_type_combobox->set_text (WAVE_TEXT_TRIANGLE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_SAW_UP)
    wave_type_combobox->set_text (WAVE_TEXT_SAW_UP);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_SAW_DOWN)
    wave_type_combobox->set_text (WAVE_TEXT_SAW_DOWN);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_SQUARE)
    wave_type_combobox->set_text (WAVE_TEXT_SQUARE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_RANDOM_SH)
    wave_type_combobox->set_text (WAVE_TEXT_RANDOM_SH);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_RANDOM_LINEAR)
    wave_type_combobox->set_text (WAVE_TEXT_RANDOM_LINEAR);
  else
    {
      g_assert_not_reached();
    }
  connect (wave_type_combobox->signal_item_changed, this, &MorphLFOView::on_wave_type_changed);

  op_layout.add_row (3, new Label (body_widget, "Wave Type"), wave_type_combobox);

  pv_frequency.init_ui (body_widget, op_layout);
  pv_depth.init_ui (body_widget, op_layout);
  pv_center.init_ui (body_widget, op_layout);
  pv_start_phase.init_ui (body_widget, op_layout);

  // FLAG: SYNC PHASE
  CheckBox *sync_voices_box = new CheckBox (body_widget, "Sync Phase for all voices");
  sync_voices_box->set_checked (morph_lfo->sync_voices());
  op_layout.add_row (2, sync_voices_box);

  connect (sync_voices_box->signal_toggled, [morph_lfo] (bool new_value) {
    morph_lfo->set_sync_voices (new_value);
  });

  op_layout.activate();
}

double
MorphLFOView::view_height()
{
  return 18;
}

void
MorphLFOView::on_wave_type_changed()
{
  string text = wave_type_combobox->text();

  if (text == WAVE_TEXT_SINE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SINE);
  else if (text == WAVE_TEXT_TRIANGLE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_TRIANGLE);
  else if (text == WAVE_TEXT_SAW_UP)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SAW_UP);
  else if (text == WAVE_TEXT_SAW_DOWN)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SAW_DOWN);
  else if (text == WAVE_TEXT_SQUARE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SQUARE);
  else if (text == WAVE_TEXT_RANDOM_SH)
    morph_lfo->set_wave_type (MorphLFO::WAVE_RANDOM_SH);
  else if (text == WAVE_TEXT_RANDOM_LINEAR)
    morph_lfo->set_wave_type (MorphLFO::WAVE_RANDOM_LINEAR);
  else
    {
      g_assert_not_reached();
    }
}
