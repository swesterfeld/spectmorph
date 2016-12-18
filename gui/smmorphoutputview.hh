// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"

#include <QComboBox>
#include <QCheckBox>

namespace SpectMorph
{

struct PropView
{
  QLabel *title;
  QSlider *slider;
  QLabel *label;

  void setVisible (bool visible);
};

class MorphOutputView : public MorphOperatorView
{
  Q_OBJECT

  struct ChannelView {
    QLabel           *label;
    ComboBoxOperator *combobox;
  };
  std::vector<ChannelView *>  channels;
  QLabel                     *unison_voices_title;
  QLabel                     *unison_voices_label;
  QSlider                    *unison_voices_slider;

  PropView pv_unison_detune;

  MorphOutput                *morph_output;

  PropView pv_adsr_skip;
  PropView pv_adsr_attack;
  PropView pv_adsr_decay;
  PropView pv_adsr_sustain;
  PropView pv_adsr_release;
public:
  MorphOutputView (MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

  void update_visibility();

public slots:
  void on_sines_changed (bool new_value);
  void on_noise_changed (bool new_value);
  void on_chorus_changed (bool new_value);
  void on_unison_voices_changed (int voices);
  void on_adsr_changed (bool new_value);
  void on_operator_changed();
};

}

#endif
