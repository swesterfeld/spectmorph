/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>
#include <bse/gsldatautils.h>
#include <bse/bseloader.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smsampleview.hh"
#include "smzoomcontroller.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  Gtk::HBox           button_hbox;
  Gtk::VBox           vbox;
  SampleView          sample_view;
  Audio               audio;
  ZoomController      zoom_controller;
  Gtk::ToggleButton   edit_clip_start;
  Gtk::ToggleButton   edit_clip_end;
  Gtk::Label          time_label;

public:
  MainWindow();

  void on_zoom_changed();
  void on_mouse_time_changed (int time);
  void load (GslDataHandle *dhandle);
};

MainWindow::MainWindow() :
  zoom_controller (1, 5000, 10, 5000)
{
  scrolled_win.add (sample_view);

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  vbox.pack_start (button_hbox, Gtk::PACK_SHRINK);

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MainWindow::on_zoom_changed));
  sample_view.signal_mouse_time_changed.connect (sigc::mem_fun (*this, &MainWindow::on_mouse_time_changed));
  on_mouse_time_changed (0); // init label

  edit_clip_start.set_label ("Edit Clip Start");
  edit_clip_end.set_label ("Edit Clip End");
  button_hbox.pack_start (time_label);
  button_hbox.pack_start (edit_clip_start);
  button_hbox.pack_start (edit_clip_end);
  add (vbox);
  show_all_children();
}

void
MainWindow::load (GslDataHandle *dhandle)
{
  gsl_data_handle_open (dhandle);
  audio.mix_freq = gsl_data_handle_mix_freq (dhandle);
  sample_view.load (dhandle, &audio);
  gsl_data_handle_close (dhandle);
}

void
MainWindow::on_zoom_changed()
{
  sample_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
MainWindow::on_mouse_time_changed (int time)
{
  int ms = time % 1000;
  time /= 1000;
  int s = time % 60;
  time /= 60;
  int m = time;
  time_label.set_label (Birnet::string_printf ("Time: %02d:%02d:%03d ms", m, s, ms));
}

GslDataHandle*
dhandle_from_file (const string& filename)
{
  /* open input */
  BseErrorType error;

  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (filename.c_str(), &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }
  return dhandle;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  if (argc != 2)
    {
      printf ("usage: %s <wave>\n", argv[0]);
      exit (1);
    }

  MainWindow main_window;
  main_window.load (dhandle_from_file (argv[1]));
  Gtk::Main::run (main_window);

  return 0;
}
