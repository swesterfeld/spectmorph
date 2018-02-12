// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include "smcheckbox.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOutputView::MorphOutputView (Widget *parent, MorphOutput *morph_output, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_output, morph_plan_window),
  morph_output (morph_output),
  morph_output_properties (morph_output)
{
  FixedGrid grid;

  source_combobox = new ComboBoxOperator (this, morph_output->morph_plan(),
    [](MorphOperator *op) {
      return (op->output_type() == MorphOperator::OUTPUT_AUDIO);
    });

  source_combobox->set_active (morph_output->channel_op (0));   // no multichannel support implemented

  connect (source_combobox->signal_item_changed, this, &MorphOutputView::on_operator_changed);

  int yoffset = 4;
  grid.add_widget (new Label (this, "Source"), 2, yoffset, 9, 3);
  grid.add_widget (source_combobox, 11, yoffset, 30, 3);

  yoffset += 3;

  CheckBox *sines_check_box = new CheckBox (this, "Enable Sine Synthesis");
  sines_check_box->set_checked (morph_output->sines());
  grid.add_widget (sines_check_box, 2, yoffset, 30, 2);

  yoffset += 2;

  CheckBox *noise_check_box = new CheckBox (this, "Enable Noise Synthesis");
  noise_check_box->set_checked (morph_output->noise());
  grid.add_widget (noise_check_box, 2, yoffset, 30, 2);

  yoffset += 2;

  CheckBox *unison_check_box = new CheckBox (this, "Enable Unison Effect");
  unison_check_box->set_checked (morph_output->unison());
  grid.add_widget (unison_check_box, 2, yoffset, 30, 2);

  yoffset += 2;

  // Unison Detune
  unison_voices_title = new Label (this, "Voices");
  unison_voices_slider = new Slider (this, 0);
  unison_voices_slider->set_int_range (2, 7);
  unison_voices_label = new Label (this, "0");

  grid.add_widget (unison_voices_title, 2, yoffset, 9, 2);
  grid.add_widget (unison_voices_slider,  11, yoffset, 25, 2);
  grid.add_widget (unison_voices_label, 37, yoffset, 5, 2);

  int unison_voices_value = morph_output->unison_voices();
  unison_voices_slider->set_int_value (unison_voices_value);
  on_unison_voices_changed (unison_voices_value);

  connect (unison_voices_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_voices_changed);

  yoffset += 2;

  // Unison Detune
  unison_detune_title = new Label (this, "Detune");
  unison_detune_slider = new Slider (this, 0);
  unison_detune_slider->set_int_range (5, 500);
  connect (unison_detune_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_detune_changed);
  unison_detune_label = new Label(this, "0");

  grid.add_widget (unison_detune_title, 2, yoffset, 9, 2);
  grid.add_widget (unison_detune_slider,  11, yoffset, 25, 2);
  grid.add_widget (unison_detune_label, 37, yoffset, 5, 2);

  const int unison_detune_value = lrint (morph_output->unison_detune() * 10);
  unison_detune_slider->set_int_value (unison_detune_value);
  on_unison_detune_changed (unison_detune_value);

  connect (unison_detune_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_detune_changed);

  yoffset += 2;

  // ADSR
  CheckBox *adsr_check_box = new CheckBox (this, "Enable custom ADSR Envelope");
  adsr_check_box->set_checked (morph_output->adsr());
  grid.add_widget (adsr_check_box, 2, yoffset, 30, 2);

  yoffset += 2;

  // Portamento (Mono): on/off
  CheckBox *portamento_check_box = new CheckBox (this, "Enable Portamento (Mono)");
  portamento_check_box->set_checked (morph_output->portamento());
  grid.add_widget (portamento_check_box, 2, yoffset, 30, 2);

  yoffset += 2;

  // Vibrato
  CheckBox *vibrato_check_box = new CheckBox (this, "Enable Vibrato");
  vibrato_check_box->set_checked (morph_output->vibrato());
  grid.add_widget (vibrato_check_box, 2, yoffset, 30, 2);

  connect (sines_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_sines (new_value);
  });
  connect (noise_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_noise (new_value);
  });
  connect (unison_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_unison (new_value);
    update_enabled();
  });
  connect (adsr_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_adsr (new_value);
    update_enabled();
  });
  connect (portamento_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_portamento (new_value);
    update_enabled();
  });
  connect (vibrato_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_vibrato (new_value);
    update_enabled();
  });

  update_enabled();
}

double
MorphOutputView::view_height()
{
  return 24;
}

void
MorphOutputView::on_operator_changed()
{
  morph_output->set_channel_op (0, source_combobox->active());
}

void
MorphOutputView::on_unison_voices_changed (int voices)
{
  unison_voices_label->text = string_locale_printf ("%d", voices);
  morph_output->set_unison_voices (voices);
}

void
MorphOutputView::on_unison_detune_changed (int new_value)
{
  const double detune = new_value / 10.0;
  unison_detune_label->text = string_locale_printf ("%.1f Cent", detune);
  morph_output->set_unison_detune (detune);
}

void
MorphOutputView::update_enabled()
{
  unison_voices_title->set_enabled (morph_output->unison());
  unison_voices_label->set_enabled (morph_output->unison());
  unison_voices_slider->set_enabled (morph_output->unison());

  unison_detune_title->set_enabled (morph_output->unison());
  unison_detune_label->set_enabled (morph_output->unison());
  unison_detune_slider->set_enabled (morph_output->unison());
}
