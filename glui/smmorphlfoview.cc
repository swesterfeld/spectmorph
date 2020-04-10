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

#define BEAT_SYNC_TEXT_OFF      "Off"
#define BEAT_SYNC_TEXT_1_1      "1/1"
#define BEAT_SYNC_TEXT_1_2      "1/2"
#define BEAT_SYNC_TEXT_1_4      "1/4"

MorphLFOView::MorphLFOView (Widget *parent, MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo),
  morph_lfo_properties (morph_lfo),
  pv_frequency (morph_lfo_properties.frequency),
  pv_depth (morph_lfo_properties.depth),
  pv_center (morph_lfo_properties.center),
  pv_start_phase (morph_lfo_properties.start_phase)
{
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

  // FREQUENCY
  pv_frequency.init_ui (body_widget, op_layout);

  // NOTE
  note_widget = new Widget (body_widget);

  for (int note = MorphLFO::NOTE_32_1; note <= MorphLFO::NOTE_1_64; note++)
    {
      string text;

      if (note <= MorphLFO::NOTE_1_1)
        text = string_printf ("%d/1", sm_round_positive (pow (2.0, MorphLFO::NOTE_32_1 - note + 5)));
      else
        text = string_printf ("1/%d", sm_round_positive (pow (2.0, note - MorphLFO::NOTE_1_1)));

      ev_note.add_item (note, text);
    }

  note_combobox = ev_note.create_combobox (note_widget, morph_lfo->note(),
    [morph_lfo] (int i) { morph_lfo->set_note (MorphLFO::Note (i)); });

  // NOTE MODE
  ev_note_mode.add_item (MorphLFO::NOTE_MODE_STRAIGHT, "straight");
  ev_note_mode.add_item (MorphLFO::NOTE_MODE_TRIPLET, "triplet");
  ev_note_mode.add_item (MorphLFO::NOTE_MODE_DOTTED, "dotted");

  ComboBox *note_mode_combobox;
  note_mode_combobox = ev_note_mode.create_combobox (note_widget, morph_lfo->note_mode(),
    [morph_lfo] (int i) { morph_lfo->set_note_mode (MorphLFO::NoteMode (i)); });

  FixedGrid grid;
  grid.add_widget (note_combobox, 0, 0, 14, 3);
  grid.add_widget (note_mode_combobox, 15, 0, 14, 3);

  note_label = new Label (body_widget, "Note");

  op_layout.add_row (3, note_label, note_widget);

  // DEPTH / CENTER / START_PHASE
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

  // FLAG: BEAT SYNC
  CheckBox *beat_sync_box = new CheckBox (body_widget, "Beat Sync");
  beat_sync_box->set_checked (morph_lfo->beat_sync());
  op_layout.add_row (2, beat_sync_box);

  connect (beat_sync_box->signal_toggled, [this, morph_lfo] (bool new_value) {
    morph_lfo->set_beat_sync (new_value);
    update_visible();
  });

  op_layout.activate();
  update_visible();
}

double
MorphLFOView::view_height()
{
  return op_layout_height + 5;
}

void
MorphLFOView::update_visible()
{
  pv_frequency.set_visible (!morph_lfo->beat_sync());
  note_label->set_visible (morph_lfo->beat_sync());
  note_widget->set_visible (morph_lfo->beat_sync());

  op_layout_height = op_layout.activate();
  signal_size_changed();
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
