// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_VIEW_HH
#define SPECTMORPH_MORPH_LINEAR_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphLinearView : public MorphOperatorView
{
protected:
  MorphLinear                     *morph_linear;

  Label                           *morphing_title;
  Label                           *morphing_label;
  Slider                          *morphing_slider;

  ComboBoxOperator                *left_combobox;
  ComboBoxOperator                *right_combobox;
  ComboBoxOperator                *control_combobox;

  void update_slider();

public:
  MorphLinearView (Widget *parent, MorphLinear *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;

/* slots: */
  void on_morphing_changed (double new_value);
  void on_control_changed();
  void on_operator_changed();
  void on_db_linear_changed (bool new_value);
};

}

#endif
