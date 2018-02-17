// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfoview.hh"
#include "smmorphplan.hh"
#include "smutils.hh"
#include "smcheckbox.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

#define WAVE_TEXT_SINE     "Sine"
#define WAVE_TEXT_TRIANGLE "Triangle"

MorphLFOView::MorphLFOView (Widget *parent, MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo),
  morph_lfo_properties (morph_lfo),
  pv_frequency (morph_lfo_properties.frequency),
  pv_depth (morph_lfo_properties.depth),
  pv_center (morph_lfo_properties.center),
  pv_start_phase (morph_lfo_properties.start_phase)
{
  FixedGrid grid;

  // WAVE TYPE
  wave_type_combobox = new ComboBox (body_widget);
  wave_type_combobox->add_item (WAVE_TEXT_SINE);
  wave_type_combobox->add_item (WAVE_TEXT_TRIANGLE);

  if (morph_lfo->wave_type() == MorphLFO::WAVE_SINE)
    wave_type_combobox->set_text (WAVE_TEXT_SINE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_TRIANGLE)
    wave_type_combobox->set_text (WAVE_TEXT_TRIANGLE);
  else
    {
      assert (false);
    }
  connect (wave_type_combobox->signal_item_changed, this, &MorphLFOView::on_wave_type_changed);

  int yoffset = 0;
  grid.add_widget (new Label (body_widget, "Wave Type"), 0, yoffset, 9, 3);
  grid.add_widget (wave_type_combobox, 9, yoffset, 30, 3);

  yoffset += 3;

  yoffset += pv_frequency.init_ui (body_widget, grid, yoffset);
  yoffset += pv_depth.init_ui (body_widget, grid, yoffset);
  yoffset += pv_center.init_ui (body_widget, grid, yoffset);
  yoffset += pv_start_phase.init_ui (body_widget, grid, yoffset);

  // FLAG: SYNC PHASE
  CheckBox *sync_voices_box = new CheckBox (body_widget, "Sync Phase for all voices");
  sync_voices_box->set_checked (morph_lfo->sync_voices());
  grid.add_widget (sync_voices_box, 0, yoffset, 30, 2);

  connect (sync_voices_box->signal_toggled, [morph_lfo] (bool new_value) {
    morph_lfo->set_sync_voices (new_value);
  });

  yoffset += 2;
}

double
MorphLFOView::view_height()
{
  return 18;
}

void
MorphLFOView::on_wave_type_changed()
{
  string text = wave_type_combobox->text();

  if (text == WAVE_TEXT_SINE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_SINE);
  else if (text == WAVE_TEXT_TRIANGLE)
    morph_lfo->set_wave_type (MorphLFO::WAVE_TRIANGLE);
  else
    {
      assert (false);
    }
}
