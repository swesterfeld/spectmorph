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

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

RenameOperatorDialog::RenameOperatorDialog (QWidget *parent, MorphOperator *op) :
  QDialog (parent),
  op (op)
{
  QLabel *old_label = new QLabel ("Old name");
  QLabel *new_label = new QLabel ("New name");
  QLabel *old_name_label = new QLabel (op->name().c_str());
  new_name_edit = new QLineEdit(op->name().c_str());
  connect (new_name_edit, SIGNAL (textChanged (QString)), this, SLOT (on_name_changed()));

  QGridLayout *grid_layout = new QGridLayout();
  grid_layout->addWidget (old_label, 0, 0);
  grid_layout->addWidget (old_name_label, 0, 1);
  grid_layout->addWidget (new_label, 1, 0);
  grid_layout->addWidget (new_name_edit, 1, 1);

  QHBoxLayout *button_layout = new QHBoxLayout;
  grid_layout->addLayout(button_layout, 2, 0, 1, 2);
  button_layout->addStretch();

  QPushButton *cancel_button = new QPushButton ("Cancel");
  connect (cancel_button, SIGNAL (clicked()), this, SLOT (reject()));
  button_layout->addWidget (cancel_button);

  ok_button = new QPushButton ("Ok");
  ok_button->setDefault (true);
  connect (ok_button, SIGNAL (clicked()), this, SLOT (accept()));
  button_layout->addWidget (ok_button);

  setLayout (grid_layout);
}

string
RenameOperatorDialog::new_name()
{
  return new_name_edit->text().toLatin1().data();
}

void
RenameOperatorDialog::on_name_changed()
{
  ok_button->setEnabled (op->can_rename (new_name()));
}
