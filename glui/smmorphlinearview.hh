// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_VIEW_HH
#define SPECTMORPH_MORPH_LINEAR_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"
#include "smcomboboxoperator.hh"
#include "smcontrolview.hh"
#include "smoperatorlayout.hh"

namespace SpectMorph
{

class MorphLinearView : public MorphOperatorView
{
protected:
  MorphLinear                     *morph_linear;

  PropertyView                    *pv_morphing;
  OperatorLayout                   op_layout;

  ComboBoxOperator                *left_combobox;
  ComboBoxOperator                *right_combobox;

public:
  MorphLinearView (Widget *parent, MorphLinear *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
  void update_visible() override;

/* slots: */
  void on_operator_changed();
  void on_db_linear_changed (bool new_value);
  void on_index_changed();
};

}

#endif
