// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_VIEW_HH
#define SPECTMORPH_MORPH_LINEAR_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"
#include "smcomboboxoperator.hh"

#include <QLabel>
#include <QSlider>
#include <QStackedWidget>

namespace SpectMorph
{

class MorphLinearView : public MorphOperatorView
{
  Q_OBJECT
protected:
  MorphLinear                     *morph_linear;

  TypeOperatorFilter               operator_filter;
  TypeOperatorFilter               control_operator_filter;
  QLabel                          *morphing_label;
  QSlider                         *morphing_slider;
  QStackedWidget                  *morphing_stack;

  ComboBoxOperator                *left_combobox;
  ComboBoxOperator                *right_combobox;
  ComboBoxOperator                *control_combobox;

  void update_slider();

public:
  MorphLinearView (MorphLinear *op, MorphPlanWindow *morph_plan_window);

public slots:
  void on_morphing_changed (int new_value);
  void on_control_changed();
  void on_operator_changed();
  void on_db_linear_changed (bool new_value);
#if SPECTMORPH_SUPPORT_LPC
  void on_use_lpc_changed (bool new_value);
#endif
};

}

#endif
