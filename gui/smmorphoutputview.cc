// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"

#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

namespace {

struct MyOperatorFilter : public OperatorFilter
{
  bool filter (MorphOperator *op)
  {
    return (op->output_type() == MorphOperator::OUTPUT_AUDIO);
  }
} op_filter_instance;

}

MorphOutputView::MorphOutputView (MorphOutput *morph_output, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_output, morph_plan_window),
  morph_output (morph_output),
  morph_output_properties (morph_output),
  pv_adsr_skip (morph_output_properties.adsr_skip),
  pv_adsr_attack (morph_output_properties.adsr_attack),
  pv_adsr_decay (morph_output_properties.adsr_decay),
  pv_adsr_sustain (morph_output_properties.adsr_sustain),
  pv_adsr_release (morph_output_properties.adsr_release),
  pv_portamento_glide (morph_output_properties.portamento_glide),
  pv_vibrato_depth (morph_output_properties.vibrato_depth),
  pv_vibrato_frequency (morph_output_properties.vibrato_frequency),
  pv_vibrato_attack (morph_output_properties.vibrato_attack)
{
  QGridLayout *grid_layout = new QGridLayout();
  grid_layout->setColumnStretch (1, 1);

  const int channel_count = SPECTMORPH_SUPPORT_MULTI_CHANNEL ? 4 : 1;
  for (int ch = 0; ch < channel_count; ch++)
    {
      ChannelView *chv = new ChannelView();
      chv->label = new QLabel (string_locale_printf ("Channel #%d", ch + 1).c_str());
      chv->combobox = new ComboBoxOperator (morph_output->morph_plan(), &op_filter_instance);

      grid_layout->addWidget (chv->label, ch, 0);
      grid_layout->addWidget (chv->combobox, ch, 1, 1, 2);
      channels.push_back (chv);

      chv->combobox->set_active (morph_output->channel_op (ch));

      connect (chv->combobox, SIGNAL (active_changed()), this, SLOT (on_operator_changed()));
    }
  QCheckBox *sines_check_box = new QCheckBox ("Enable Sine Synthesis");
  sines_check_box->setChecked (morph_output->sines());
  grid_layout->addWidget (sines_check_box, 4, 0, 1, 2);

  QCheckBox *noise_check_box = new QCheckBox ("Enable Noise Synthesis");
  noise_check_box->setChecked (morph_output->noise());
  grid_layout->addWidget (noise_check_box, 5, 0, 1, 2);

  QCheckBox *unison_check_box = new QCheckBox ("Enable Unison Effect");
  unison_check_box->setChecked (morph_output->unison());
  grid_layout->addWidget (unison_check_box, 6, 0, 1, 2);

  // Unison Voices
  unison_voices_slider = new QSlider (Qt::Horizontal);
  unison_voices_slider->setRange (2, 7);
  connect (unison_voices_slider, SIGNAL (valueChanged (int)), this, SLOT (on_unison_voices_changed (int)));
  unison_voices_label = new QLabel();
  unison_voices_title = new QLabel ("Voices");

  grid_layout->addWidget (unison_voices_title, 7, 0);
  grid_layout->addWidget (unison_voices_slider, 7, 1);
  grid_layout->addWidget (unison_voices_label, 7, 2);

  int unison_voices_value = morph_output->unison_voices();
  unison_voices_slider->setValue (unison_voices_value);
  on_unison_voices_changed (unison_voices_value);

  // Unison Detune
  unison_detune_slider = new QSlider (Qt::Horizontal);
  unison_detune_slider->setRange (5, 500);
  connect (unison_detune_slider, SIGNAL (valueChanged (int)), this, SLOT (on_unison_detune_changed (int)));
  unison_detune_label = new QLabel();
  unison_detune_title = new QLabel ("Detune");

  grid_layout->addWidget (unison_detune_title, 8, 0);
  grid_layout->addWidget (unison_detune_slider, 8, 1);
  grid_layout->addWidget (unison_detune_label, 8, 2);

  const int unison_detune_value = lrint (morph_output->unison_detune() * 10);
  unison_detune_slider->setValue (unison_detune_value);
  on_unison_detune_changed (unison_detune_value);

  // ADSR
  QCheckBox *adsr_check_box = new QCheckBox ("Enable custom ADSR Envelope");
  adsr_check_box->setChecked (morph_output->adsr());
  grid_layout->addWidget (adsr_check_box, 9, 0, 1, 2);

  pv_adsr_skip.init_ui (grid_layout, 10);
  pv_adsr_attack.init_ui (grid_layout, 11);
  pv_adsr_decay.init_ui (grid_layout, 12);
  pv_adsr_sustain.init_ui (grid_layout, 13);
  pv_adsr_release.init_ui (grid_layout, 14);

  // Portamento (Mono): on/off
  QCheckBox *portamento_check_box = new QCheckBox ("Enable Portamento (Mono)");
  portamento_check_box->setChecked (morph_output->portamento());
  grid_layout->addWidget (portamento_check_box, 15, 0, 1, 2);

  pv_portamento_glide.init_ui (grid_layout, 16);

  // Vibrato
  QCheckBox *vibrato_check_box = new QCheckBox ("Enable Vibrato");
  vibrato_check_box->setChecked (morph_output->vibrato());
  grid_layout->addWidget (vibrato_check_box, 17, 0, 1, 2);

  pv_vibrato_depth.init_ui (grid_layout, 18);
  pv_vibrato_frequency.init_ui (grid_layout, 19);
  pv_vibrato_attack.init_ui (grid_layout, 20);

  // Unison, ADSR, Portamento, Vibrato: initial widget visibility
  update_visibility();

  connect (sines_check_box, SIGNAL (toggled (bool)), this, SLOT (on_sines_changed (bool)));
  connect (noise_check_box, SIGNAL (toggled (bool)), this, SLOT (on_noise_changed (bool)));
  connect (unison_check_box, SIGNAL (toggled (bool)), this, SLOT (on_unison_changed (bool)));
  connect (adsr_check_box, SIGNAL (toggled (bool)), this, SLOT (on_adsr_changed (bool)));
  connect (portamento_check_box, SIGNAL (toggled (bool)), this, SLOT (on_portamento_changed (bool)));
  connect (vibrato_check_box, SIGNAL (toggled (bool)), this, SLOT (on_vibrato_changed (bool)));

  set_body_layout (grid_layout);
}

void
MorphOutputView::on_sines_changed (bool new_value)
{
  morph_output->set_sines (new_value);
}

void
MorphOutputView::on_noise_changed (bool new_value)
{
  morph_output->set_noise (new_value);
}

void
MorphOutputView::on_unison_changed (bool new_value)
{
  morph_output->set_unison (new_value);

  update_visibility();

  Q_EMIT need_resize();
}

void
MorphOutputView::on_unison_voices_changed (int voices)
{
  unison_voices_label->setText (string_locale_printf ("%d", voices).c_str());
  morph_output->set_unison_voices (voices);
}

void
MorphOutputView::on_unison_detune_changed (int new_value)
{
  const double detune = new_value / 10.0;
  unison_detune_label->setText (string_locale_printf ("%.1f Cent", detune).c_str());
  morph_output->set_unison_detune (detune);
}

void
MorphOutputView::on_adsr_changed (bool new_value)
{
  morph_output->set_adsr (new_value);

  update_visibility();

  Q_EMIT need_resize();
}

void
MorphOutputView::on_portamento_changed (bool new_value)
{
  morph_output->set_portamento (new_value);

  update_visibility();

  Q_EMIT need_resize();
}

void
MorphOutputView::on_vibrato_changed (bool new_value)
{
  morph_output->set_vibrato (new_value);

  update_visibility();

  Q_EMIT need_resize();
}

void
MorphOutputView::update_visibility()
{
  unison_voices_title->setVisible (morph_output->unison());
  unison_voices_label->setVisible (morph_output->unison());
  unison_voices_slider->setVisible (morph_output->unison());

  unison_detune_title->setVisible (morph_output->unison());
  unison_detune_label->setVisible (morph_output->unison());
  unison_detune_slider->setVisible (morph_output->unison());

  pv_adsr_skip.set_visible (morph_output->adsr());
  pv_adsr_attack.set_visible (morph_output->adsr());
  pv_adsr_decay.set_visible (morph_output->adsr());
  pv_adsr_sustain.set_visible (morph_output->adsr());
  pv_adsr_release.set_visible (morph_output->adsr());

  pv_portamento_glide.set_visible (morph_output->portamento());

  pv_vibrato_depth.set_visible (morph_output->vibrato());
  pv_vibrato_frequency.set_visible (morph_output->vibrato());
  pv_vibrato_attack.set_visible (morph_output->vibrato());
}

void
MorphOutputView::on_operator_changed()
{
  for (size_t i = 0; i < channels.size(); i++)
    {
      morph_output->set_channel_op (i, channels[i]->combobox->active());
    }
}
