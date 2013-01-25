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

#include <QComboBox>
#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceView::MorphSourceView (MorphSource *morph_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_source, morph_plan_window),
  morph_source (morph_source)
{
  QLabel *instrument_label = new QLabel ("Instrument");
  instrument_combobox = new QComboBox();

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->addWidget (instrument_label);
  hbox->addWidget (instrument_combobox);

  frame_group_box->setLayout (hbox);

  on_index_changed();
  morph_source->morph_plan()->signal_index_changed.connect (sigc::mem_fun (*this, &MorphSourceView::on_index_changed));

  connect (instrument_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_instrument_changed()));
}

void
MorphSourceView::on_index_changed()
{
  instrument_combobox->clear();

  vector<string> smsets = morph_source->morph_plan()->index()->smsets();
  for (vector<string>::iterator si = smsets.begin(); si != smsets.end(); si++)
    instrument_combobox->addItem ((*si).c_str());

  int index = instrument_combobox->findText (morph_source->smset().c_str());
  if (index >= 0)
    instrument_combobox->setCurrentIndex (index);
}

void
MorphSourceView::on_instrument_changed()
{
  morph_source->set_smset (instrument_combobox->currentText().toLatin1().data());
}

#if 0
MorphSourceView::MorphSourceView (MorphSource *morph_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_source, morph_plan_window),
  instrument_label ("Instrument"),
  morph_source (morph_source)
{
  on_index_changed();
  morph_source->morph_plan()->signal_index_changed.connect (sigc::mem_fun (*this, &MorphSourceView::on_index_changed));

  instrument_hbox.set_spacing (10);
  instrument_hbox.pack_start (instrument_label, Gtk::PACK_SHRINK);
  instrument_hbox.pack_start (instrument_combobox);
  instrument_hbox.set_border_width (5);
  frame.add (instrument_hbox);

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
#endif
