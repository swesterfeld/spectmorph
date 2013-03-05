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

#include "smdisplayparamwindow.hh"
#include <assert.h>
#include <birnet/birnet.hh>

#include <QVBoxLayout>

using namespace SpectMorph;

DisplayParamWindow::DisplayParamWindow()
{
  setWindowTitle ("Display Parameters");

  QVBoxLayout *vbox = new QVBoxLayout();

  show_lpc_checkbox = new QCheckBox ("Show LPC Envelope in Spectrum View");
  show_lsf_checkbox = new QCheckBox ("Show LPC LSF Parameters in Spectrum View");

  connect (show_lpc_checkbox, SIGNAL (clicked()), this, SLOT (on_param_changed()));
  connect (show_lsf_checkbox, SIGNAL (clicked()), this, SLOT (on_param_changed()));

  vbox->addWidget (show_lpc_checkbox);
  vbox->addWidget (show_lsf_checkbox);

  setLayout (vbox);
}

void
DisplayParamWindow::on_param_changed()
{
  emit params_changed();
}

bool
DisplayParamWindow::show_lsf()
{
  return show_lsf_checkbox->isChecked();
}

bool
DisplayParamWindow::show_lpc()
{
  return show_lpc_checkbox->isChecked();
}
