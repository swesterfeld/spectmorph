// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
  // WAVE TYPE
  ev_wave_type.add_item (MorphLFO::WAVE_SINE,           "Sine");
  ev_wave_type.add_item (MorphLFO::WAVE_TRIANGLE,       "Triangle");
  ev_wave_type.add_item (MorphLFO::WAVE_SAW_UP,         "Saw Up");
  ev_wave_type.add_item (MorphLFO::WAVE_SAW_DOWN,       "Saw Down");
  ev_wave_type.add_item (MorphLFO::WAVE_SQUARE,         "Square");
  ev_wave_type.add_item (MorphLFO::WAVE_RANDOM_SH,      "Random Sample & Hold");
  ev_wave_type.add_item (MorphLFO::WAVE_RANDOM_LINEAR,  "Random Linear");

  ComboBox *wave_type_combobox;
  wave_type_combobox = ev_wave_type.create_combobox (body_widget, morph_lfo->wave_type(),
    [morph_lfo] (int i) { morph_lfo->set_wave_type (MorphLFO::WaveType (i)); });

  op_layout.add_row (3, new Label (body_widget, "Wave Type"), wave_type_combobox);

  // FREQUENCY
  pv_frequency = add_property_view (MorphLFO::P_FREQUENCY, op_layout);

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

  ComboBox *note_combobox;
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

  op_layout.activate();
  signal_size_changed();
}
