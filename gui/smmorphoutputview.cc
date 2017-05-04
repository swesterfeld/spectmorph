// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include <functional>

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

void
PropView::setVisible (bool visible)
{
  title->setVisible (visible);
  slider->setVisible (visible);
  label->setVisible (visible);
}

PropView
xprop (QGridLayout *grid_layout, int row,
       const std::string& title, float min, float max, int decimal_factor, const std::string& fmt,
       MorphOutput *morph_op,
       std::function<float(const MorphOutput&)> get,
       std::function<void (MorphOutput&, float)> set)
{
  PropView pv;

  pv.title = new QLabel (title.c_str());

  pv.slider = new QSlider (Qt::Horizontal);
  pv.slider->setRange (lrint (min * decimal_factor), lrint (max * decimal_factor));

  const int value = lrint (get (*morph_op) * decimal_factor);
  pv.slider->setValue (value);

  pv.label = new QLabel();
  pv.label->setText (string_locale_printf (fmt.c_str(), double (value) / decimal_factor).c_str());

  QObject::connect (pv.slider, &QSlider::valueChanged,
    [=](int value) {
      float f = double (value) / decimal_factor;
      pv.label->setText (string_locale_printf (fmt.c_str(), f).c_str());
      set (*morph_op, f);
    });

  grid_layout->addWidget (pv.title, row, 0);
  grid_layout->addWidget (pv.slider, row, 1);
  grid_layout->addWidget (pv.label, row, 2);

  return pv;
}

MorphOutputView::MorphOutputView (MorphOutput *morph_output, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_output, morph_plan_window),
  morph_output (morph_output)
{
  QGridLayout *grid_layout = new QGridLayout();
  grid_layout->setColumnStretch (1, 1);

  for (int ch = 0; ch < 4; ch++)
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

  pv_adsr_skip = xprop (grid_layout, 10,   "Skip",      0, 500, 10, "%.1f ms", morph_output, &MorphOutput::adsr_skip,     &MorphOutput::set_adsr_skip);
  pv_adsr_attack = xprop (grid_layout, 11,   "Attack",    0, 100, 10, "%.1f %%", morph_output, &MorphOutput::adsr_attack,   &MorphOutput::set_adsr_attack);
  pv_adsr_decay = xprop (grid_layout, 12,   "Decay",     0, 100, 10, "%.1f %%", morph_output, &MorphOutput::adsr_decay,    &MorphOutput::set_adsr_decay);
  pv_adsr_sustain = xprop (grid_layout, 13,   "Sustain",   0, 100, 10, "%.1f %%", morph_output, &MorphOutput::adsr_sustain,  &MorphOutput::set_adsr_sustain);
  pv_adsr_release = xprop (grid_layout, 14,   "Release",   0, 100, 10, "%.1f %%", morph_output, &MorphOutput::adsr_release,  &MorphOutput::set_adsr_release);

  // Unison, ADSR: initial widget visibility
  update_visibility();

  connect (sines_check_box, SIGNAL (toggled (bool)), this, SLOT (on_sines_changed (bool)));
  connect (noise_check_box, SIGNAL (toggled (bool)), this, SLOT (on_noise_changed (bool)));
  connect (unison_check_box, SIGNAL (toggled (bool)), this, SLOT (on_unison_changed (bool)));
  connect (adsr_check_box, SIGNAL (toggled (bool)), this, SLOT (on_adsr_changed (bool)));
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
MorphOutputView::update_visibility()
{
  unison_voices_title->setVisible (morph_output->unison());
  unison_voices_label->setVisible (morph_output->unison());
  unison_voices_slider->setVisible (morph_output->unison());

  unison_detune_title->setVisible (morph_output->unison());
  unison_detune_label->setVisible (morph_output->unison());
  unison_detune_slider->setVisible (morph_output->unison());

  pv_adsr_skip.setVisible (morph_output->adsr());
  pv_adsr_attack.setVisible (morph_output->adsr());
  pv_adsr_decay.setVisible (morph_output->adsr());
  pv_adsr_sustain.setVisible (morph_output->adsr());
  pv_adsr_release.setVisible (morph_output->adsr());
}

void
MorphOutputView::on_operator_changed()
{
  for (size_t i = 0; i < channels.size(); i++)
    {
      morph_output->set_channel_op (i, channels[i]->combobox->active());
    }
}
