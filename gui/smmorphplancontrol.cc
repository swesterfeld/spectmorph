// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplancontrol.hh"
#include "smutils.hh"

#include <QGridLayout>

using namespace SpectMorph;

using std::string;

MorphPlanControl::MorphPlanControl (MorphPlanPtr plan, Features f) :
  morph_plan (plan),
  volume_value_label (nullptr),
  volume_slider (nullptr),
  midi_led (nullptr),
  inst_status (nullptr)
{
  QGridLayout *grid = new QGridLayout (this);
  if (f == ALL_WIDGETS)
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

      grid->addWidget (volume_label, 0, 0);
      grid->addWidget (volume_slider, 0, 1);
      grid->addWidget (volume_value_label, 0, 2);
      grid->addWidget (midi_led, 0, 3);

      volume_value_label->setMinimumSize (volume_label->fontMetrics().boundingRect ("-XX.X dB").size());
      volume_value_label->setAlignment (Qt::AlignRight | Qt::AlignVCenter);

      inst_status = new QLabel();
      grid->addWidget (inst_status, 1, 0, 1, 4);
    }
  else
    {
      inst_status = new QLabel();
      grid->addWidget (inst_status);
    }
  setLayout (grid);
  setTitle ("Global Instrument Settings");

  connect (plan.c_ptr(), SIGNAL (index_changed()), this, SLOT (on_index_changed()));

  on_index_changed();
}

void
MorphPlanControl::set_volume (double v_db)
{
  volume_slider->setValue (qBound (-480, qRound (v_db * 10), 120));
}

void
MorphPlanControl::set_led (bool on)
{
  midi_led->setState (on ? Led::On : Led::Off);
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
      inst_status->setToolTip (QString ("Instrument Set is \"%1\".\nThis Instrument Set filename is \"%2\".") .
                                        arg (morph_plan->index()->dir().c_str()) .
                                        arg (morph_plan->index()->expanded_filename().c_str()));
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
      inst_status->setToolTip (QString ("Custom Instrument Set is \"%1\".").arg (morph_plan->index()->filename().c_str()));
    }
  if (morph_plan->index()->type() == INDEX_NOT_DEFINED)
    {
      red = true;
      text = string_printf ("NEED TO LOAD Instrument Set.");
      inst_status->setToolTip ("Instrument Set is empty.");
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
