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

using namespace SpectMorph;

#if 0
DisplayParamWindow::DisplayParamWindow()
{
  set_border_width (10);
  set_title ("Display Parameters");

  show_lpc_button.set_label ("Show LPC Envelope in Spectrum View");
  show_lpc_button.signal_toggled().connect (sigc::mem_fun (*this, &DisplayParamWindow::on_param_changed));
  vbox.pack_start (show_lpc_button, Gtk::PACK_SHRINK);

  show_lsf_button.set_label ("Show LPC LSF Parameters in Spectrum View");
  show_lsf_button.signal_toggled().connect (sigc::mem_fun (*this, &DisplayParamWindow::on_param_changed));
  vbox.pack_start (show_lsf_button, Gtk::PACK_SHRINK);

  add (vbox);

  show_all_children();
}

void
DisplayParamWindow::on_param_changed()
{
  signal_params_changed();
}

bool
DisplayParamWindow::show_lsf()
{
  return show_lsf_button.get_active();
}

bool
DisplayParamWindow::show_lpc()
{
  return show_lpc_button.get_active();
}
#endif
