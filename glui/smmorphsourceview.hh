// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphsource.hh"
#include "smcombobox.hh"
#include "smmorphplanwindow.hh"

namespace SpectMorph
{

class MorphSourceView : public MorphOperatorView
{
  MorphSource      *morph_source;
  ComboBox         *instrument_combobox;

public:
  MorphSourceView (Widget *parent, MorphSource *morph_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;

  void on_index_changed();
  void on_instrument_changed();
};

}

#endif
