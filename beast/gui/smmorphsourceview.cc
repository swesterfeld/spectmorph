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

#include "smmorphsourceview.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceView::MorphSourceView (MorphSource *morph_source) :
  instrument_label ("Instrument"),
  morph_source (morph_source)
{
  set_label ("Source");

  on_index_changed();
  morph_source->morph_plan()->signal_index_changed.connect (sigc::mem_fun (*this, &MorphSourceView::on_index_changed));

  instrument_hbox.set_spacing (10);
  instrument_hbox.pack_start (instrument_label, Gtk::PACK_SHRINK);
  instrument_hbox.pack_start (instrument_combobox);
  instrument_hbox.set_border_width (5);
  add (instrument_hbox);

  instrument_combobox.signal_changed().connect (sigc::mem_fun (*this, &MorphSourceView::on_instrument_changed));

  show_all_children();
}

void
MorphSourceView::on_index_changed()
{
  instrument_combobox.clear();

  vector<string> smsets = morph_source->morph_plan()->index()->smsets();
  for (vector<string>::iterator si = smsets.begin(); si != smsets.end(); si++)
    {
      instrument_combobox.append_text (*si);
      if (*si == morph_source->smset())
        instrument_combobox.set_active_text (*si);
    }
}

void
MorphSourceView::on_instrument_changed()
{
  morph_source->set_smset (instrument_combobox.get_active_text());
}
