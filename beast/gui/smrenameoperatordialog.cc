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

#include "smrenameoperatordialog.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

RenameOperatorDialog::RenameOperatorDialog (MorphOperator *op) :
  op (op)
{
  Gtk::Box& box = *get_vbox();

  old_label.set_label ("Old name");
  old_name_label.set_label (op->name());
  old_name_label.set_alignment (0, 0);

  new_label.set_label ("New name");
  new_name_entry.set_text (op->name());
  new_name_entry.property_text().signal_changed().connect (sigc::mem_fun (*this, &RenameOperatorDialog::on_name_changed));

  table.attach (old_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (old_name_label, 1, 2, 0, 1);
  table.attach (new_label, 0, 1, 1, 2, Gtk::SHRINK);
  table.attach (new_name_entry, 1, 2, 1, 2);

  table.set_spacings (10);
  table.set_border_width (5);

  box.add (table);

  ok_button = add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  show_all_children();
}

string
RenameOperatorDialog::new_name()
{
  return new_name_entry.get_text();
}

void
RenameOperatorDialog::on_name_changed()
{
  ok_button->set_sensitive (op->can_rename (new_name_entry.get_text()));
}
