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

#ifndef SPECTMORPH_MORPH_LFO_VIEW_HH
#define SPECTMORPH_MORPH_LFO_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphlfo.hh"
#include "smcomboboxoperator.hh"

namespace SpectMorph
{

class MorphLFOView : public MorphOperatorView
{
  Q_OBJECT

  MorphLFO  *morph_lfo;

  QComboBox *wave_type_combobox;

  QLabel    *frequency_label;
  QLabel    *depth_label;
  QLabel    *center_label;
  QLabel    *start_phase_label;

public:
  MorphLFOView (MorphLFO *op, MorphPlanWindow *morph_plan_window);

public slots:
  void on_wave_type_changed();
  void on_frequency_changed (int new_value);
  void on_depth_changed (int new_value);
  void on_center_changed (int new_value);
  void on_start_phase_changed (int new_value);
  void on_sync_voices_changed (bool new_value);
};

}

#endif
