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

  add_property_view (MorphOutput::P_VELOCITY_SENSITIVITY, op_layout);

  add_property_view (MorphOutput::P_SINES, op_layout);
  add_property_view (MorphOutput::P_NOISE, op_layout);

  // Unison
  pv_unison        = add_property_view (MorphOutput::P_UNISON, op_layout);
  pv_unison_voices = add_property_view (MorphOutput::P_UNISON_VOICES, op_layout);
  pv_unison_detune = add_property_view (MorphOutput::P_UNISON_DETUNE, op_layout);

  // ADSR
  pv_adsr = add_property_view (MorphOutput::P_ADSR, op_layout);

  // ADSR Widget
  output_adsr_widget = new OutputADSRWidget (body_widget, morph_output, this);
  op_layout.add_fixed (30, 8, output_adsr_widget);

  pv_adsr_skip = add_property_view (MorphOutput::P_ADSR_SKIP, op_layout);
  pv_adsr_attack = add_property_view (MorphOutput::P_ADSR_ATTACK, op_layout);
  pv_adsr_decay = add_property_view (MorphOutput::P_ADSR_DECAY, op_layout);
  pv_adsr_sustain = add_property_view (MorphOutput::P_ADSR_SUSTAIN, op_layout);
  pv_adsr_release = add_property_view (MorphOutput::P_ADSR_RELEASE, op_layout);

  // Filter
  pv_filter = add_property_view (MorphOutput::P_FILTER, op_layout);
  pv_filter_type = add_property_view (MorphOutput::P_FILTER_TYPE, op_layout);
  pv_filter_attack = add_property_view (MorphOutput::P_FILTER_ATTACK, op_layout);
  pv_filter_decay = add_property_view (MorphOutput::P_FILTER_DECAY, op_layout);
  pv_filter_sustain = add_property_view (MorphOutput::P_FILTER_SUSTAIN, op_layout);
  pv_filter_release = add_property_view (MorphOutput::P_FILTER_RELEASE, op_layout);
  pv_filter_depth = add_property_view (MorphOutput::P_FILTER_DEPTH, op_layout);
  pv_filter_key_tracking = add_property_view (MorphOutput::P_FILTER_KEY_TRACKING, op_layout);
  pv_filter_cutoff = add_property_view (MorphOutput::P_FILTER_CUTOFF, op_layout);
  pv_filter_resonance = add_property_view (MorphOutput::P_FILTER_RESONANCE, op_layout);
  pv_filter_drive = add_property_view (MorphOutput::P_FILTER_DRIVE, op_layout);
  pv_filter_mix = add_property_view (MorphOutput::P_FILTER_MIX, op_layout);

  // Portamento (Mono): on/off
  pv_portamento = add_property_view (MorphOutput::P_PORTAMENTO, op_layout);
  pv_portamento_glide = add_property_view (MorphOutput::P_PORTAMENTO_GLIDE, op_layout);

  // Vibrato
  pv_vibrato = add_property_view (MorphOutput::P_VIBRATO, op_layout);
  pv_vibrato_depth = add_property_view (MorphOutput::P_VIBRATO_DEPTH, op_layout);
  pv_vibrato_frequency = add_property_view (MorphOutput::P_VIBRATO_FREQUENCY, op_layout);
  pv_vibrato_attack = add_property_view (MorphOutput::P_VIBRATO_ATTACK, op_layout);

  // visibility updates
  for (auto pv : { pv_unison, pv_adsr, pv_filter, pv_portamento, pv_vibrato })
    connect (pv->property()->signal_value_changed, this, &MorphOutputView::update_visible);

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
  bool unison = pv_unison->property()->get_bool();
  pv_unison_voices->set_visible (unison);
  pv_unison_detune->set_visible (unison);

  bool adsr = pv_adsr->property()->get_bool();
  output_adsr_widget->set_visible (adsr);

  pv_adsr_skip->set_visible (adsr);
  pv_adsr_attack->set_visible (adsr);
  pv_adsr_decay->set_visible (adsr);
  pv_adsr_sustain->set_visible (adsr);
  pv_adsr_release->set_visible (adsr);

  bool filter = pv_filter->property()->get_bool();
  pv_filter_type->set_visible (filter);
  pv_filter_attack->set_visible (filter);
  pv_filter_decay->set_visible (filter);
  pv_filter_sustain->set_visible (filter);
  pv_filter_release->set_visible (filter);
  pv_filter_depth->set_visible (filter);
  pv_filter_key_tracking->set_visible (filter);
  pv_filter_cutoff->set_visible (filter);
  pv_filter_resonance->set_visible (filter);
  pv_filter_drive->set_visible (filter);
  pv_filter_mix->set_visible (filter);

  bool portamento = pv_portamento->property()->get_bool();
  pv_portamento_glide->set_visible (portamento);

  bool vibrato = pv_vibrato->property()->get_bool();
  pv_vibrato_depth->set_visible (vibrato);
  pv_vibrato_frequency->set_visible (vibrato);
  pv_vibrato_attack->set_visible (vibrato);

  op_layout.activate();
  signal_size_changed();
}
