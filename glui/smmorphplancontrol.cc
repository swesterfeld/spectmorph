// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplancontrol.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"

using namespace SpectMorph;

using std::string;

MorphPlanControl::MorphPlanControl (Widget *parent, MorphPlanPtr plan, Features f) :
  Frame (parent),
  morph_plan (plan)
{
  FixedGrid grid;

  Label *title_label = new Label (this, "Global Instrument Settings");
  title_label->align = TextAlign::CENTER;
  title_label->bold  = true;

  grid.add_widget (title_label, 0, 0, 43, 4);

  int voffset = 4;

  volume_slider = new Slider (this, 0);
  volume_value_label = new Label (this, "");
  midi_led = new Led (this, false);

  connect (volume_slider->signal_value_changed, this, &MorphPlanControl::on_volume_changed);

  grid.add_widget (new Label (this, "Volume"), 2, voffset, 7, 2);
  grid.add_widget (volume_slider, 8, voffset, 23, 2);
  grid.add_widget (volume_value_label, 32, voffset, 7, 2);
  grid.add_widget (midi_led, 39, voffset, 2, 2);

  voffset += 2;

  // start at -6 dB
  set_volume (-6.0);

  // initial label
  on_volume_changed (volume_slider->value);

  inst_status = new Label (this, "");
  grid.add_widget (inst_status, 2, voffset, 40, 2);

  connect (plan->signal_index_changed, this, &MorphPlanControl::on_index_changed);

  on_index_changed();
}

void
MorphPlanControl::set_volume (double v_db)
{
  volume_slider->value = (v_db + 48) / 60; // map [-48:12] -> [0:1]
}

void
MorphPlanControl::on_volume_changed (double new_volume)
{
  double new_volume_f = new_volume * 60 - 48; // map [0:1] -> [-48:12]
  volume_value_label->text = string_locale_printf ("%.1f dB", new_volume_f);

  signal_volume_changed (new_volume_f); // emit dB value
}

void
MorphPlanControl::set_led (bool on)
{
  midi_led->set_on (on);
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
#if 0
      inst_status->setToolTip (QString ("Instrument Set is \"%1\".\nThis Instrument Set filename is \"%2\".") .
                                        arg (morph_plan->index()->dir().c_str()) .
                                        arg (morph_plan->index()->expanded_filename().c_str()));
#endif
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
#if 0
      inst_status->setToolTip (QString ("Custom Instrument Set is \"%1\".").arg (morph_plan->index()->filename().c_str()));
#endif
    }
  if (morph_plan->index()->type() == INDEX_NOT_DEFINED)
    {
      red = true;
      text = string_printf ("NEED TO LOAD Instrument Set.");
#if 0
      inst_status->setToolTip ("Instrument Set is empty.");
#endif
    }
  if (red)
    inst_status->color = Color (1.0, 0.0, 0.0);
  else
    inst_status->color = ThemeColor::TEXT;
  inst_status->text = text;
}


