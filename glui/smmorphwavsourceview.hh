// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphwavsource.hh"
#include "smmorphplanwindow.hh"
#include "smcombobox.hh"
#include "smprogressbar.hh"
#include "smenumview.hh"
#include "smpropertyview.hh"
#include "smcomboboxoperator.hh"
#include "smoperatorlayout.hh"
#include "smcontrolview.hh"

namespace SpectMorph
{

class MorphWavSourceView : public MorphOperatorView
{
  MorphWavSource   *morph_wav_source = nullptr;
  ComboBox         *instrument_combobox = nullptr;
  ProgressBar      *progress_bar = nullptr;
  Label            *instrument_label = nullptr;
  Label            *position_control_input_label = nullptr;
  std::unique_ptr<Instrument> edit_instrument; // temporary copy used for editing

  MorphWavSourceProperties morph_wav_source_properties;

  PropertyView      pv_position;
  EnumView          ev_play_mode;
  ControlView       cv_position_control;
  ComboBoxOperator *position_control_combobox;
  OperatorLayout    op_layout;

  void on_edit();
  void on_instrument_changed();
  void on_update_progress();

  std::string modified_check (bool& wav_source_update, bool& user_int_update);

  void update_instrument_list();
  void update_visible();
  void on_edit_close();
  void on_edit_save_changes (bool save_changes);
  void on_position_control_changed();
public:
  MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
