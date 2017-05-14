// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"
#include "smpropertyview.hh"

#include <QComboBox>
#include <QCheckBox>

namespace SpectMorph
{

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

  QLabel                     *unison_detune_title;
  QLabel                     *unison_detune_label;
  QSlider                    *unison_detune_slider;

  MorphOutput                *morph_output;

  PropertyView                pv_portamento_glide;
public:
  MorphOutputView (MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

public slots:
  void on_sines_changed (bool new_value);
  void on_noise_changed (bool new_value);
  void on_unison_changed (bool new_value);
  void on_unison_voices_changed (int voices);
  void on_unison_detune_changed (int new_value);
  void on_portamento_changed (bool new_value);
  void on_operator_changed();
};

}

#endif
