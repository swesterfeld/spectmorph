// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplancontrol.hh"
#include "smutils.hh"

#include <QGridLayout>

using namespace SpectMorph;

using std::string;

MorphPlanControl::MorphPlanControl (MorphPlanPtr plan) :
  morph_plan (plan)
{
  QLabel *volume_label = new QLabel ("Volume", this);
  volume_slider = new QSlider (Qt::Horizontal, this);
  volume_slider->setRange (-480, 120);
  volume_value_label = new QLabel (this);
  connect (volume_slider, SIGNAL (valueChanged(int)), this, SLOT (on_volume_changed(int)));

  // start at -6 dB
  set_volume (-6.0);

  midi_led = new Led();
  midi_led->off();

  QGridLayout *grid = new QGridLayout (this);
  grid->addWidget (volume_label, 0, 0);
  grid->addWidget (volume_slider, 0, 1);
  grid->addWidget (volume_value_label, 0, 2);
  grid->addWidget (midi_led, 0, 3);

  inst_status = new QLabel();
  grid->addWidget (inst_status, 1, 0, 1, 4);

  setLayout (grid);
  setTitle ("Global Instrument Settings");

#if 0
  connect (synth, SIGNAL (voices_active_changed()), this, SLOT (on_update_led()));
#endif

  connect (plan.c_ptr(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
  connect (plan.c_ptr(), SIGNAL (index_changed()), this, SLOT (on_index_changed()));

  on_index_changed();
  on_plan_changed();
}

void
MorphPlanControl::set_volume (double v_db)
{
  volume_slider->setValue (qBound (-480, qRound (v_db * 10), 120));
}

void
MorphPlanControl::on_plan_changed()
{
}

void
MorphPlanControl::on_index_changed()
{
  string text;
  bool red = false;

  if (morph_plan->index()->type() == INDEX_INSTRUMENTS_DIR)
    {
      if (morph_plan->index()->load_ok())
        {
          text = string_printf ("Loaded '%s' Instrument Set.", morph_plan->index()->dir().c_str());
        }
      else
        {
          red = true;
          text = string_printf ("Instrument Set '%s' NOT FOUND.", morph_plan->index()->dir().c_str());
        }
    }
  if (morph_plan->index()->type() == INDEX_FILENAME)
    {
      if (morph_plan->index()->load_ok())
        {
          text = string_printf ("Loaded Custom Instrument Set.");
        }
      else
        {
          red = true;
          text = string_printf ("Custom Instrument Set NOT FOUND.");
        }
    }
  if (morph_plan->index()->type() == INDEX_NOT_DEFINED)
    {
      red = true;
      text = string_printf ("NEED TO LOAD Instrument Set.");
    }
  if (red)
    {
      text = "<font color='darkred'>" + text + "</font>";
    }
  inst_status->setText (text.c_str());
}

void
MorphPlanControl::on_volume_changed (int new_volume)
{
  double new_volume_f = new_volume * 0.1;
  volume_value_label->setText (string_locale_printf ("%.1f dB", new_volume_f).c_str());

  Q_EMIT change_volume (new_volume_f); // emit dB value
}
