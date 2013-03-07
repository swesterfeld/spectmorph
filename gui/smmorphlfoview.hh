// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
  Q_OBJECT

  MorphLFO  *morph_lfo;

  QComboBox *wave_type_combobox;

  QLabel    *frequency_label;
  QLabel    *depth_label;
  QLabel    *center_label;
  QLabel    *start_phase_label;

public:
  MorphLFOView (MorphLFO *op, MorphPlanWindow *morph_plan_window);

public slots:
  void on_wave_type_changed();
  void on_frequency_changed (int new_value);
  void on_depth_changed (int new_value);
  void on_center_changed (int new_value);
  void on_start_phase_changed (int new_value);
  void on_sync_voices_changed (bool new_value);
};

}

#endif
