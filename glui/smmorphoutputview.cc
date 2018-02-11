// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"

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

//  sines_check_box = new CheckBox ("Enable Sine Synthesis");
}

double
MorphOutputView::view_height()
{
  return 8;
}

void
MorphOutputView::on_operator_changed()
{
  morph_output->set_channel_op (0, source_combobox->active());
}
