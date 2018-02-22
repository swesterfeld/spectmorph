// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"

namespace SpectMorph
{

class MorphOutputView : public MorphOperatorView
{
  ComboBoxOperator           *source_combobox;

  Label                      *unison_voices_title;
  Label                      *unison_voices_label;
  Slider                     *unison_voices_slider;

  Label                      *unison_detune_title;
  Label                      *unison_detune_label;
  Slider                     *unison_detune_slider;

  MorphOutput                *morph_output;

  MorphOutputProperties       morph_output_properties;

  PropertyView                pv_adsr_skip;
  PropertyView                pv_adsr_attack;
  PropertyView                pv_adsr_decay;
  PropertyView                pv_adsr_sustain;
  PropertyView                pv_adsr_release;

  PropertyView                pv_portamento_glide;
  PropertyView                pv_vibrato_depth;
  PropertyView                pv_vibrato_frequency;
  PropertyView                pv_vibrato_attack;

  void update_enabled();

public:
  MorphOutputView (Widget *parent, MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

  double view_height() override;
  bool   is_output() override;

  /* slots */
  void on_operator_changed();
  void on_unison_voices_changed (int voices);
  void on_unison_detune_changed (int detune);
};

}

#endif
