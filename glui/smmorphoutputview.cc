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
  connect (unison_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_unison (new_value);
  });
  connect (adsr_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_adsr (new_value);
  });
  connect (portamento_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_portamento (new_value);
  });
  connect (vibrato_check_box->signal_toggled, [morph_output] (bool new_value) {
    morph_output->set_vibrato (new_value);
  });
}

double
MorphOutputView::view_height()
{
  return 20;
}

void
MorphOutputView::on_operator_changed()
{
  morph_output->set_channel_op (0, source_combobox->active());
}
