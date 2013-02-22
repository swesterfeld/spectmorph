/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include <jack/jack.h>
#include <jack/midiport.h>

#if 0
int velocity = 100;

class MainWindow : public Gtk::Window
{
  Gtk::Adjustment velocity_adjustment;
  Gtk::HScale     velocity_scale;
public:
  MainWindow();
  void value_changed();
};

MainWindow::MainWindow() :
  velocity_adjustment (100.0, 0.0, 127.0, 0.1, 1.0, 0.0),
  velocity_scale (velocity_adjustment)
{
  set_border_width (10);
  add (velocity_scale);
  velocity_scale.set_size_request (320, 40);
  velocity_scale.show();
  velocity_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWindow::value_changed));
}

void
MainWindow::value_changed()
{
  velocity = (int) (velocity_adjustment.get_value() + 0.5);
}

jack_port_t *input_port;
jack_port_t *output_port;

static bool
is_note_on (const jack_midi_event_t& event)
{
  if ((event.buffer[0] & 0xf0) == 0x90)
    {
      if (event.buffer[2] != 0) /* note on with velocity 0 => note off */
        return true;
    }
  return false;
}

int
jack_process (jack_nframes_t nframes, void *)
{
  void* port_buf = jack_port_get_buffer (input_port, nframes);
  void* out_port_buf = jack_port_get_buffer (output_port, nframes);
  jack_nframes_t event_count = jack_midi_get_event_count (port_buf);

  jack_midi_clear_buffer (out_port_buf);

  for (size_t i = 0; i < event_count; i++)
    {
      jack_midi_event_t in_event;
      jack_midi_event_get (&in_event, port_buf, i);

      if (is_note_on (in_event))
        {
          unsigned char my_event[3];
          my_event[0] = in_event.buffer[0];
          my_event[1] = in_event.buffer[1];
          my_event[2] = velocity;
          jack_midi_event_write (out_port_buf, in_event.time, my_event, 3);
        }
      else
        {
          jack_midi_event_write (out_port_buf, in_event.time, in_event.buffer, in_event.size); // midi through
        }
    }
  return 0;
}

#endif

int
main (int argc, char **argv)
{
#if 0
  Gtk::Main kit (argc, argv);

  jack_client_t *client;
  client = jack_client_open ("smjackvel", JackNullOption, NULL);

  input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  output_port = jack_port_register (client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

  jack_set_process_callback (client, jack_process, 0);

  if (jack_activate (client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }

  MainWindow window;

  Gtk::Main::run (window);
#endif
}
