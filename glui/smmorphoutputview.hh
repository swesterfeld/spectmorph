// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphOutputView : public MorphOperatorView
{
  ComboBoxOperator           *source_combobox;

  MorphOutput                *morph_output;

  MorphOutputProperties       morph_output_properties;

public:
  MorphOutputView (Widget *parent, MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

  double view_height() override;

  /* slots */
  void on_operator_changed();
};

}

#endif
