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

#ifndef SPECTMORPH_MORPH_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphsource.hh"

namespace SpectMorph
{

class MorphSourceView : public MorphOperatorView
{
  Gtk::HBox         instrument_hbox;
  Gtk::Label        instrument_label;
  Gtk::ComboBoxText instrument_combobox;
  MorphSource      *morph_source;

public:
  MorphSourceView (MorphSource *morph_source, MorphPlanWindow *morph_plan_window);

  void on_index_changed();
  void on_instrument_changed();
};

}

#endif
