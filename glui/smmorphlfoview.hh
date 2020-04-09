// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
  MorphLFO  *morph_lfo;

  MorphLFOProperties       morph_lfo_properties;

  ComboBox  *wave_type_combobox;
  ComboBox  *note_combobox;

  PropertyView pv_frequency;
  PropertyView pv_depth;
  PropertyView pv_center;
  PropertyView pv_start_phase;

  std::string note2text (int note);

public:
  MorphLFOView (Widget *widget, MorphLFO *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;

/* slots: */
  void on_wave_type_changed();
  void on_note_changed();
};

}

#endif
