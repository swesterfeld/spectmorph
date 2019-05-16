// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphwavsource.hh"
#include "smmorphplanwindow.hh"
#include "smcombobox.hh"

namespace SpectMorph
{

class MorphWavSourceView : public MorphOperatorView
{
  MorphWavSource   *morph_wav_source = nullptr;
  ComboBox         *instrument_combobox = nullptr;

  void on_load();
  void on_edit();
  void on_instrument_changed();

  void update_instrument_list();
  void write_instrument();
public:
  MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
