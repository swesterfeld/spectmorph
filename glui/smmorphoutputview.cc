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
  morph_output (morph_output)
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
  add_property_view (MorphOutput::P_VELOCITY_SENSITIVITY, body_widget, op_layout);

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

  pv_unison_voices = add_property_view (MorphOutput::P_UNISON_VOICES, body_widget, op_layout);
  pv_unison_detune = add_property_view (MorphOutput::P_UNISON_DETUNE, body_widget, op_layout);

  // ADSR
  CheckBox *adsr_check_box = new CheckBox (body_widget, "Enable custom ADSR Envelope");
  adsr_check_box->set_checked (morph_output->adsr());
  op_layout.add_row (2, adsr_check_box);

  // ADSR Widget
  output_adsr_widget = new OutputADSRWidget (body_widget, morph_output, this);
  op_layout.add_fixed (30, 8, output_adsr_widget);

  pv_adsr_skip = add_property_view (MorphOutput::P_ADSR_SKIP, body_widget, op_layout);
  pv_adsr_attack = add_property_view (MorphOutput::P_ADSR_ATTACK, body_widget, op_layout);
  pv_adsr_decay = add_property_view (MorphOutput::P_ADSR_DECAY, body_widget, op_layout);
  pv_adsr_sustain = add_property_view (MorphOutput::P_ADSR_SUSTAIN, body_widget, op_layout);
  pv_adsr_release = add_property_view (MorphOutput::P_ADSR_RELEASE, body_widget, op_layout);

  // Filter
  CheckBox *filter_check_box = new CheckBox (body_widget, "Enable Filter");
  filter_check_box->set_checked (morph_output->filter());
  op_layout.add_row (2, filter_check_box);

  pv_filter_attack = add_property_view (MorphOutput::P_FILTER_ATTACK, body_widget, op_layout);
  pv_filter_decay = add_property_view (MorphOutput::P_FILTER_DECAY, body_widget, op_layout);
  pv_filter_sustain = add_property_view (MorphOutput::P_FILTER_SUSTAIN, body_widget, op_layout);
  pv_filter_release = add_property_view (MorphOutput::P_FILTER_RELEASE, body_widget, op_layout);
  pv_filter_depth = add_property_view (MorphOutput::P_FILTER_DEPTH, body_widget, op_layout);
  pv_filter_cutoff = add_property_view (MorphOutput::P_FILTER_CUTOFF, body_widget, op_layout);
  pv_filter_resonance = add_property_view (MorphOutput::P_FILTER_RESONANCE, body_widget, op_layout);

  // Portamento (Mono): on/off
  CheckBox *portamento_check_box = new CheckBox (body_widget, "Enable Portamento (Mono)");
  portamento_check_box->set_checked (morph_output->portamento());
  op_layout.add_row (2, portamento_check_box);

  pv_portamento_glide = add_property_view (MorphOutput::P_PORTAMENTO_GLIDE, body_widget, op_layout);

  // Vibrato
  CheckBox *vibrato_check_box = new CheckBox (body_widget, "Enable Vibrato");
  vibrato_check_box->set_checked (morph_output->vibrato());
  op_layout.add_row (2, vibrato_check_box);

  pv_vibrato_depth = add_property_view (MorphOutput::P_VIBRATO_DEPTH, body_widget, op_layout);
  pv_vibrato_frequency = add_property_view (MorphOutput::P_VIBRATO_FREQUENCY, body_widget, op_layout);
  pv_vibrato_attack = add_property_view (MorphOutput::P_VIBRATO_ATTACK, body_widget, op_layout);

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
  connect (filter_check_box->signal_toggled, [=] (bool new_value) {
    morph_output->set_filter (new_value);
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
MorphOutputView::update_visible()
{
  pv_unison_voices->set_visible (morph_output->unison());
  pv_unison_detune->set_visible (morph_output->unison());

  output_adsr_widget->set_visible (morph_output->adsr());

  pv_adsr_skip->set_visible (morph_output->adsr());
  pv_adsr_attack->set_visible (morph_output->adsr());
  pv_adsr_decay->set_visible (morph_output->adsr());
  pv_adsr_sustain->set_visible (morph_output->adsr());
  pv_adsr_release->set_visible (morph_output->adsr());

  pv_filter_attack->set_visible (morph_output->filter());
  pv_filter_decay->set_visible (morph_output->filter());
  pv_filter_sustain->set_visible (morph_output->filter());
  pv_filter_release->set_visible (morph_output->filter());
  pv_filter_depth->set_visible (morph_output->filter());
  pv_filter_cutoff->set_visible (morph_output->filter());
  pv_filter_resonance->set_visible (morph_output->filter());

  pv_portamento_glide->set_visible (morph_output->portamento());

  pv_vibrato_depth->set_visible (morph_output->vibrato());
  pv_vibrato_frequency->set_visible (morph_output->vibrato());
  pv_vibrato_attack->set_visible (morph_output->vibrato());

  op_layout.activate();
  signal_size_changed();
}
