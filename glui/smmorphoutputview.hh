// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"
#include "smoperatorlayout.hh"
#include "smoutputadsrwidget.hh"

namespace SpectMorph
{

class MorphOutputView : public MorphOperatorView
{
  ComboBoxOperator           *source_combobox;

  MorphOutput                *morph_output;

  PropertyView               *pv_velocity_sensitivity;

  PropertyView               *pv_unison;
  PropertyView               *pv_unison_voices;
  PropertyView               *pv_unison_detune;

  PropertyView               *pv_adsr;
  PropertyView               *pv_adsr_skip;
  PropertyView               *pv_adsr_attack;
  PropertyView               *pv_adsr_decay;
  PropertyView               *pv_adsr_sustain;
  PropertyView               *pv_adsr_release;

  PropertyView               *pv_filter;
  PropertyView               *pv_filter_type;
  PropertyView               *pv_filter_attack;
  PropertyView               *pv_filter_decay;
  PropertyView               *pv_filter_sustain;
  PropertyView               *pv_filter_release;
  PropertyView               *pv_filter_depth;
  PropertyView               *pv_filter_cutoff;
  PropertyView               *pv_filter_resonance;

  PropertyView               *pv_portamento;
  PropertyView               *pv_portamento_glide;

  PropertyView               *pv_vibrato;
  PropertyView               *pv_vibrato_depth;
  PropertyView               *pv_vibrato_frequency;
  PropertyView               *pv_vibrato_attack;

  OutputADSRWidget           *output_adsr_widget;

  OperatorLayout              op_layout;

  void update_visible();

public:
  MorphOutputView (Widget *parent, MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

  double view_height() override;
  bool   is_output() override;

  /* slots */
  void on_operator_changed();
};

}

#endif
