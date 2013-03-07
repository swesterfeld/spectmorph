// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphsource.hh"

#include <QComboBox>

namespace SpectMorph
{

class MorphSourceView : public MorphOperatorView
{
  Q_OBJECT

  MorphSource      *morph_source;
  QComboBox        *instrument_combobox;

public:
  MorphSourceView (MorphSource *morph_source, MorphPlanWindow *morph_plan_window);

public slots:
  void on_index_changed();
  void on_instrument_changed();
};

}

#endif
