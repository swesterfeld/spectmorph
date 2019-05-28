// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphwavsource.hh"
#include "smmorphplanwindow.hh"
#include "smcombobox.hh"
#include "smprogressbar.hh"

namespace SpectMorph
{

class MorphWavSourceView : public MorphOperatorView
{
  MorphWavSource   *morph_wav_source = nullptr;
  ComboBox         *instrument_combobox = nullptr;
  ProgressBar      *progress_bar = nullptr;
  Label            *instrument_label = nullptr;

  void on_edit();
  void on_instrument_changed();
  void on_update_progress();

  void update_instrument_list();
  void write_instrument();
public:
  MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
