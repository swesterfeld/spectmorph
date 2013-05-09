// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfoview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"

#include <QLabel>
#include <QCheckBox>

using namespace SpectMorph;

using std::string;
using std::vector;

#define WAVE_TEXT_SINE     "Sine"
#define WAVE_TEXT_TRIANGLE "Triangle"

MorphLFOView::MorphLFOView (MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo)
{
  QGridLayout *grid_layout = new QGridLayout();

  // WAVE TYPE
  grid_layout->addWidget (new QLabel ("Wave Type"), 0, 0);
  wave_type_combobox = new QComboBox();
  wave_type_combobox->addItem (WAVE_TEXT_SINE);
  wave_type_combobox->addItem (WAVE_TEXT_TRIANGLE);
  grid_layout->addWidget (wave_type_combobox, 0, 1, 1, 2);

  if (morph_lfo->wave_type() == MorphLFO::WAVE_SINE)
    wave_type_combobox->setCurrentText (WAVE_TEXT_SINE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_TRIANGLE)
    wave_type_combobox->setCurrentText (WAVE_TEXT_TRIANGLE);
  else
    {
      assert (false);
    }
  connect (wave_type_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_wave_type_changed()));

  // FREQUENCY
  QSlider *frequency_slider = new QSlider (Qt::Horizontal);
  frequency_slider->setRange (-200, 100);
  connect (frequency_slider, SIGNAL (valueChanged (int)), this, SLOT (on_frequency_changed (int)));
  frequency_label = new QLabel();

  grid_layout->addWidget (new QLabel ("Frequency"), 1, 0);
  grid_layout->addWidget (frequency_slider, 1, 1);
  grid_layout->addWidget (frequency_label, 1, 2);

  int frequency_value = lrint (log10 (morph_lfo->frequency()) * 100);
  frequency_slider->setValue (frequency_value);
  on_frequency_changed (frequency_value);

  // DEPTH
  QSlider *depth_slider = new QSlider (Qt::Horizontal);
  depth_slider->setRange (0, 100);
  connect (depth_slider, SIGNAL (valueChanged (int)), this, SLOT (on_depth_changed (int)));
  depth_label = new QLabel();

  grid_layout->addWidget (new QLabel ("Depth"), 2, 0);
  grid_layout->addWidget (depth_slider, 2, 1);
  grid_layout->addWidget (depth_label, 2, 2);

  int depth_value = lrint (morph_lfo->depth() * 100);
  depth_slider->setValue (depth_value);
  on_depth_changed (depth_value);

  // CENTER
  QSlider *center_slider = new QSlider (Qt::Horizontal);
  center_slider->setRange (-100, 100);
  connect (center_slider, SIGNAL (valueChanged (int)), this, SLOT (on_center_changed (int)));
  center_label = new QLabel();

  grid_layout->addWidget (new QLabel ("Center"), 3, 0);
  grid_layout->addWidget (center_slider, 3, 1);
  grid_layout->addWidget (center_label, 3, 2);

  int center_value = lrint (morph_lfo->center() * 100);
  center_slider->setValue (center_value);
  on_center_changed (center_value);

  // START PHASE
  QSlider *start_phase_slider = new QSlider (Qt::Horizontal);
  start_phase_slider->setRange (-180, 180);
  connect (start_phase_slider, SIGNAL (valueChanged (int)), this, SLOT (on_start_phase_changed (int)));
  start_phase_label = new QLabel();

  int start_phase_value = lrint (morph_lfo->start_phase());
  start_phase_slider->setValue (start_phase_value);
  on_start_phase_changed (start_phase_value);

  grid_layout->addWidget (new QLabel ("Start Phase"), 4, 0);
  grid_layout->addWidget (start_phase_slider, 4, 1);
  grid_layout->addWidget (start_phase_label, 4, 2);

  // FLAG: SYNC PHASE
  QCheckBox *sync_voices_box = new QCheckBox ("Sync Phase for all voices");
  sync_voices_box->setChecked (morph_lfo->sync_voices());
  grid_layout->addWidget (sync_voices_box, 5, 0, 1, 3);
  connect (sync_voices_box, SIGNAL (toggled (bool)), this, SLOT (on_sync_voices_changed (bool)));

  setLayout (grid_layout);
}

void
MorphLFOView::on_wave_type_changed()
{
  string text = wave_type_combobox->currentText().toLatin1().data();

  if (text == WAVE_TEXT_SINE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SINE);
  else if (text == WAVE_TEXT_TRIANGLE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_TRIANGLE);
  else
    {
      assert (false);
    }
}

void
MorphLFOView::on_frequency_changed (int new_value)
{
  double frequency = pow (10, new_value / 100.0);
  frequency_label->setText (string_printf ("%.3f Hz", frequency).c_str());
  morph_lfo->set_frequency (frequency);
}

void
MorphLFOView::on_depth_changed (int new_value)
{
  double depth = new_value / 100.0;
  depth_label->setText (string_printf ("%.1f %%", depth * 100).c_str());
  morph_lfo->set_depth (depth);
}

void
MorphLFOView::on_center_changed (int new_value)
{
  double center = new_value / 100.0;
  center_label->setText (string_printf ("%.2f", center).c_str());
  morph_lfo->set_center (center);
}

void
MorphLFOView::on_start_phase_changed (int new_value)
{
  double start_phase = new_value;
  start_phase_label->setText (string_printf ("%.2f", start_phase).c_str());
  morph_lfo->set_start_phase (start_phase);
}

void
MorphLFOView::on_sync_voices_changed (bool new_value)
{
  morph_lfo->set_sync_voices (new_value);
}
