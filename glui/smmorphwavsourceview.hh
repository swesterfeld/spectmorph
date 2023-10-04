// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphwavsource.hh"
#include "smmorphplanwindow.hh"
#include "smcombobox.hh"
#include "smprogressbar.hh"
#include "smpropertyview.hh"
#include "smcomboboxoperator.hh"
#include "smoperatorlayout.hh"
#include "smcontrolview.hh"

namespace SpectMorph
{

class MorphWavSourceView : public MorphOperatorView
{
  MorphWavSource   *morph_wav_source = nullptr;
  ComboBox         *bank_combobox = nullptr;
  ComboBox         *instrument_combobox = nullptr;
  ProgressBar      *progress_bar = nullptr;
  Label            *instrument_label = nullptr;
  UserInstrumentIndex *user_instrument_index = nullptr;
  std::unique_ptr<Instrument> edit_instrument; // temporary copy used for editing

  Property         *prop_play_mode;
  PropertyView     *pv_position;
  OperatorLayout    op_layout;

  void on_edit();
  void on_edit_banks();
  void on_instrument_changed();
  void on_bank_changed();
  void on_update_progress();

  std::string modified_check (bool& wav_source_update, bool& user_int_update);

  void update_instrument_list();
  void update_visible() override;
  void on_edit_close();
  void on_edit_save_changes (bool save_changes);
  void on_banks_changed();
public:
  MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
