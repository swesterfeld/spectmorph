// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputview.hh"
#include "smoperatorlayout.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include "smcheckbox.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOutputView::MorphOutputView (Widget *parent, MorphOutput *morph_output, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_output, morph_plan_window),
  morph_output (morph_output),
  morph_output_properties (morph_output),
  pv_adsr_skip (morph_output_properties.adsr_skip),
  pv_adsr_attack (morph_output_properties.adsr_attack),
  pv_adsr_decay (morph_output_properties.adsr_decay),
  pv_adsr_sustain (morph_output_properties.adsr_sustain),
  pv_adsr_release (morph_output_properties.adsr_release),
  pv_portamento_glide (morph_output_properties.portamento_glide),
  pv_vibrato_depth (morph_output_properties.vibrato_depth),
  pv_vibrato_frequency (morph_output_properties.vibrato_frequency),
  pv_vibrato_attack (morph_output_properties.vibrato_attack),
  pv_velocity_sensitivity (morph_output_properties.velocity_sensitivity)
{
  hide_tool_buttons(); // no fold/close for output

  source_combobox = new ComboBoxOperator (body_widget, morph_output->morph_plan(),
    [](MorphOperator *op) {
      return (op->output_type() == MorphOperator::OUTPUT_AUDIO);
    });

  source_combobox->set_active (morph_output->channel_op (0));   // no multichannel support implemented

  connect (source_combobox->signal_item_changed, this, &MorphOutputView::on_operator_changed);

  op_layout.add_row (3, new Label (body_widget, "Source"), source_combobox);

  // Velocity Sensitivity
  pv_velocity_sensitivity.init_ui (body_widget, op_layout);

  // Sines + Noise
  CheckBox *sines_check_box = new CheckBox (body_widget, "Enable Sine Synthesis");
  sines_check_box->set_checked (morph_output->sines());
  op_layout.add_row (2, sines_check_box);

  CheckBox *noise_check_box = new CheckBox (body_widget, "Enable Noise Synthesis");
  noise_check_box->set_checked (morph_output->noise());
  op_layout.add_row (2, noise_check_box);

  // Unison
  CheckBox *unison_check_box = new CheckBox (body_widget, "Enable Unison Effect");
  unison_check_box->set_checked (morph_output->unison());
  op_layout.add_row (2, unison_check_box);

  // Unison Voices
  unison_voices_title = new Label (body_widget, "Voices");
  unison_voices_slider = new Slider (body_widget, 0);
  unison_voices_slider->set_int_range (2, 7);
  unison_voices_label = new Label (body_widget, "0");

  op_layout.add_row (2, unison_voices_title, unison_voices_slider, unison_voices_label);

  int unison_voices_value = morph_output->unison_voices();
  unison_voices_slider->set_int_value (unison_voices_value);
  on_unison_voices_changed (unison_voices_value);

  connect (unison_voices_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_voices_changed);


  // Unison Detune
  unison_detune_title = new Label (body_widget, "Detune");
  unison_detune_slider = new Slider (body_widget, 0);
  unison_detune_slider->set_int_range (5, 500);
  connect (unison_detune_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_detune_changed);
  unison_detune_label = new Label (body_widget, "0");

  op_layout.add_row (2, unison_detune_title, unison_detune_slider, unison_detune_label);

  const int unison_detune_value = lrint (morph_output->unison_detune() * 10);
  unison_detune_slider->set_int_value (unison_detune_value);
  on_unison_detune_changed (unison_detune_value);

  connect (unison_detune_slider->signal_int_value_changed, this, &MorphOutputView::on_unison_detune_changed);


  // ADSR
  CheckBox *adsr_check_box = new CheckBox (body_widget, "Enable custom ADSR Envelope");
  adsr_check_box->set_checked (morph_output->adsr());
  op_layout.add_row (2, adsr_check_box);

  // ADSR Widget
  output_adsr_widget = new OutputADSRWidget (body_widget, morph_output, this);
  op_layout.add_fixed (30, 8, output_adsr_widget);

  pv_adsr_skip.init_ui (body_widget, op_layout);
  pv_adsr_attack.init_ui (body_widget, op_layout);
  pv_adsr_decay.init_ui (body_widget, op_layout);
  pv_adsr_sustain.init_ui (body_widget, op_layout);
  pv_adsr_release.init_ui (body_widget, op_layout);

  // link values for property views and adsr widget
  for (auto pv_ptr : vector<PropertyView *> { &pv_adsr_attack, &pv_adsr_decay, &pv_adsr_sustain, &pv_adsr_release })
    {
      connect (pv_ptr->signal_value_changed, output_adsr_widget, &OutputADSRWidget::on_adsr_params_changed);
      connect (output_adsr_widget->signal_adsr_params_changed, pv_ptr, &PropertyView::on_update_value);
    }

  // Portamento (Mono): on/off
  CheckBox *portamento_check_box = new CheckBox (body_widget, "Enable Portamento (Mono)");
  portamento_check_box->set_checked (morph_output->portamento());
  op_layout.add_row (2, portamento_check_box);

  pv_portamento_glide.init_ui (body_widget, op_layout);

  // Vibrato
  CheckBox *vibrato_check_box = new CheckBox (body_widget, "Enable Vibrato");
  vibrato_check_box->set_checked (morph_output->vibrato());
  op_layout.add_row (2, vibrato_check_box);

  pv_vibrato_depth.init_ui (body_widget, op_layout);
  pv_vibrato_frequency.init_ui (body_widget, op_layout);
  pv_vibrato_attack.init_ui (body_widget, op_layout);

  connect (sines_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_sines (new_value);
  });
  connect (noise_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_noise (new_value);
  });
  connect (unison_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_unison (new_value);
    update_visible();
  });
  connect (adsr_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_adsr (new_value);
    update_visible();
  });
  connect (portamento_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_portamento (new_value);
    update_visible();
  });
  connect (vibrato_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_vibrato (new_value);
    update_visible();
  });

  update_visible();
}

double
MorphOutputView::view_height()
{
  return op_layout.height() + 5;
}

bool
MorphOutputView::is_output()
{
  return true;
}

void
MorphOutputView::on_operator_changed()
{
  morph_output->set_channel_op (0, source_combobox->active());
}

void
MorphOutputView::on_unison_voices_changed (int voices)
{
  unison_voices_label->set_text (string_locale_printf ("%d", voices));
  morph_output->set_unison_voices (voices);
}

void
MorphOutputView::on_unison_detune_changed (int new_value)
{
  const double detune = new_value / 10.0;
  unison_detune_label->set_text (string_locale_printf ("%.1f Cent", detune));
  morph_output->set_unison_detune (detune);
}

void
MorphOutputView::update_visible()
{
  unison_voices_title->set_visible (morph_output->unison());
  unison_voices_label->set_visible (morph_output->unison());
  unison_voices_slider->set_visible (morph_output->unison());

  unison_detune_title->set_visible (morph_output->unison());
  unison_detune_label->set_visible (morph_output->unison());
  unison_detune_slider->set_visible (morph_output->unison());

  output_adsr_widget->set_visible (morph_output->adsr());

  pv_adsr_skip.set_visible (morph_output->adsr());
  pv_adsr_attack.set_visible (morph_output->adsr());
  pv_adsr_decay.set_visible (morph_output->adsr());
  pv_adsr_sustain.set_visible (morph_output->adsr());
  pv_adsr_release.set_visible (morph_output->adsr());

  pv_portamento_glide.set_visible (morph_output->portamento());

  pv_vibrato_depth.set_visible (morph_output->vibrato());
  pv_vibrato_frequency.set_visible (morph_output->vibrato());
  pv_vibrato_attack.set_visible (morph_output->vibrato());

  op_layout.activate();
  signal_size_changed();
}
