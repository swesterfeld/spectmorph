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

#include "smsamplewindow.hh"
#include "smnavigator.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include "smlivedecoder.hh"

#include <iostream>
#include <jack/jack.h>

using namespace SpectMorph;

using std::vector;
using Birnet::AutoLocker;

#if 0
PlayerWindow::PlayerWindow (Navigator *navigator) :
  navigator (navigator),
  volume_scale (-96, 24, 0.01),
  jack_player ("sminspector")
{
  set_border_width (10);
  set_default_size (300, 100);
  set_title ("Player");

  play_button.set_label ("Play");
  play_button.signal_clicked().connect (sigc::mem_fun (*this, &PlayerWindow::on_play_clicked));

  stop_button.set_label ("Stop");
  stop_button.signal_clicked().connect (sigc::mem_fun (*this, &PlayerWindow::on_stop_clicked));

  volume_scale.signal_value_changed().connect (sigc::mem_fun (*this, &PlayerWindow::on_volume_changed));

  button_hbox.pack_start (play_button);
  button_hbox.pack_start (stop_button);
  button_hbox.set_spacing (10);

  volume_label.set_label ("Volume");
  volume_scale.set_value (0);
  volume_scale.set_draw_value (false);

  volume_hbox.pack_start (volume_label, Gtk::PACK_SHRINK);
  volume_hbox.pack_start (volume_scale);
  volume_hbox.pack_start (volume_value_label, Gtk::PACK_SHRINK);

  vbox.add (button_hbox);
  vbox.add (volume_hbox);

  add (vbox);

  show_all_children();
}

PlayerWindow::~PlayerWindow()
{
}

void
PlayerWindow::on_play_clicked()
{
  Audio *audio = navigator->get_audio();

  jack_player.play (audio, !navigator->spectmorph_signal_active());
}

void
PlayerWindow::on_stop_clicked()
{
  jack_player.play (NULL, true);
}

void
PlayerWindow::on_volume_changed()
{
  double new_decoder_volume = bse_db_to_factor (volume_scale.get_value());
  volume_value_label.set_text (Birnet::string_printf ("%.1f dB", volume_scale.get_value()));

  jack_player.set_volume (new_decoder_volume);
}
#endif
