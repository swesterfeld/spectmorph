/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphlfoview.hh"
#include "smmorphplan.hh"
#include <birnet/birnet.hh>

using namespace SpectMorph;

using std::string;
using std::vector;

#define WAVE_TEXT_SINE     "Sine"
#define WAVE_TEXT_TRIANGLE "Triangle"

MorphLFOView::MorphLFOView (MorphLFO *morph_lfo, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_lfo, morph_plan_window),
  morph_lfo (morph_lfo),
  frequency_scale (-2, 1, 0.0001),
  depth_scale (0, 1, 0.0001),
  center_scale (-1, 1, 0.0001),
  start_phase_scale (-180, 180, 0.0001)
{
  wave_type_combobox.append_text (WAVE_TEXT_SINE);
  wave_type_combobox.append_text (WAVE_TEXT_TRIANGLE);
  wave_type_label.set_text ("Wave Type");

  if (morph_lfo->wave_type() == MorphLFO::WAVE_SINE)
    wave_type_combobox.set_active_text (WAVE_TEXT_SINE);
  else if (morph_lfo->wave_type() == MorphLFO::WAVE_TRIANGLE)
    wave_type_combobox.set_active_text (WAVE_TEXT_TRIANGLE);
  else
    {
      assert (false);
    }

  table.attach (wave_type_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (wave_type_combobox, 1, 3, 0, 1);

  frequency_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLFOView::on_frequency_changed));
  frequency_label.set_text ("Frequency");
  frequency_scale.set_draw_value (false);
  frequency_scale.set_value (log10 (morph_lfo->frequency()));

  table.attach (frequency_label, 0, 1, 1, 2, Gtk::SHRINK);
  table.attach (frequency_scale, 1, 2, 1, 2);
  table.attach (frequency_value_label, 2, 3, 1, 2, Gtk::SHRINK);

  depth_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLFOView::on_depth_changed));
  depth_label.set_text ("Depth");
  depth_scale.set_draw_value (false);
  depth_scale.set_value (morph_lfo->depth());

  table.attach (depth_label, 0, 1, 2, 3, Gtk::SHRINK);
  table.attach (depth_scale, 1, 2, 2, 3);
  table.attach (depth_value_label, 2, 3, 2, 3, Gtk::SHRINK);

  center_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLFOView::on_center_changed));
  center_label.set_text ("Center");
  center_scale.set_draw_value (false);
  center_scale.set_value (morph_lfo->center());

  table.attach (center_label, 0, 1, 3, 4, Gtk::SHRINK);
  table.attach (center_scale, 1, 2, 3, 4);
  table.attach (center_value_label, 2, 3, 3, 4, Gtk::SHRINK);

  start_phase_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphLFOView::on_start_phase_changed));
  start_phase_label.set_text ("Start Phase");
  start_phase_scale.set_draw_value (false);
  start_phase_scale.set_value (morph_lfo->start_phase());

  table.attach (start_phase_label, 0, 1, 4, 5, Gtk::SHRINK);
  table.attach (start_phase_scale, 1, 2, 4, 5);
  table.attach (start_phase_value_label, 2, 3, 4, 5, Gtk::SHRINK);

  sync_voices_check_button.set_active (morph_lfo->sync_voices());
  sync_voices_check_button.set_label ("Sync phase for all voices");

  table.attach (sync_voices_check_button, 0, 2, 5, 6);

  table.set_spacings (10);
  table.set_border_width (5);

  frame.add (table);

  wave_type_combobox.signal_changed().connect (sigc::mem_fun (*this, &MorphLFOView::on_wave_type_changed));
  sync_voices_check_button.signal_toggled().connect (sigc::mem_fun (*this, &MorphLFOView::on_sync_voices_changed));

  show_all_children();
}

MorphLFOView::~MorphLFOView()
{
}

void
MorphLFOView::on_wave_type_changed()
{
  string text = wave_type_combobox.get_active_text();

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
MorphLFOView::on_frequency_changed()
{
  double frequency = pow (10, frequency_scale.get_value());
  frequency_value_label.set_text (Birnet::string_printf ("%.3f Hz", frequency));
  morph_lfo->set_frequency (frequency);
}

void
MorphLFOView::on_depth_changed()
{
  double depth = depth_scale.get_value();
  depth_value_label.set_text (Birnet::string_printf ("%.1f %%", depth * 100));
  morph_lfo->set_depth (depth);
}

void
MorphLFOView::on_center_changed()
{
  double center = center_scale.get_value();
  center_value_label.set_text (Birnet::string_printf ("%.2f", center));
  morph_lfo->set_center (center);
}

void
MorphLFOView::on_start_phase_changed()
{
  double start_phase = start_phase_scale.get_value();
  start_phase_value_label.set_text (Birnet::string_printf ("%.2f", start_phase));
  morph_lfo->set_start_phase (start_phase);
}

void
MorphLFOView::on_sync_voices_changed()
{
  morph_lfo->set_sync_voices (sync_voices_check_button.get_active());
}
