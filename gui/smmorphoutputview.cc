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

  // Unison: initial widget visibility
  on_unison_changed (morph_output->unison());

  connect (sines_check_box, SIGNAL (toggled (bool)), this, SLOT (on_sines_changed (bool)));
  connect (noise_check_box, SIGNAL (toggled (bool)), this, SLOT (on_noise_changed (bool)));
  connect (unison_check_box, SIGNAL (toggled (bool)), this, SLOT (on_unison_changed (bool)));
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

  unison_voices_title->setVisible (new_value);
  unison_voices_label->setVisible (new_value);
  unison_voices_slider->setVisible (new_value);

  unison_detune_title->setVisible (new_value);
  unison_detune_label->setVisible (new_value);
  unison_detune_slider->setVisible (new_value);

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
MorphOutputView::on_operator_changed()
{
  for (size_t i = 0; i < channels.size(); i++)
    {
      morph_output->set_channel_op (i, channels[i]->combobox->active());
    }
}
