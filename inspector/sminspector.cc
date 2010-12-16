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

#include <gtkmm.h>
#include <assert.h>
#include <bse/bseloader.h>
#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include "smmicroconf.hh"
#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smspectrumwindow.hh"
#include "smtimefreqview.hh"
#include "smpixelarray.hh"
#include "smnavigator.hh"

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  TimeFreqView        time_freq_view;
  ZoomController      zoom_controller;
  Gtk::Adjustment     position_adjustment;
  Gtk::HScale         position_scale;
  Gtk::Label          position_label;
  Gtk::HBox           position_hbox;
  Gtk::VBox           vbox;
  Navigator           navigator;
  SpectrumWindow      spectrum_window;

public:
  MainWindow (const string& filename);

  void on_zoom_changed();
  void on_dhandle_changed();
  void on_position_changed();
};

MainWindow::MainWindow (const string& filename) :
  //time_freq_view (filename),
  position_adjustment (0.0, 0.0, 1.0, 0.01, 1.0, 0.0),
  position_scale (position_adjustment),
  navigator (filename)
{
  set_border_width (10);
  set_default_size (800, 600);
  vbox.pack_start (scrolled_win);

  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MainWindow::on_zoom_changed));

  vbox.pack_start (position_hbox, Gtk::PACK_SHRINK);
  position_hbox.pack_start (position_scale);
  position_hbox.pack_start (position_label, Gtk::PACK_SHRINK);
  position_scale.set_draw_value (false);
  position_label.set_text ("frame 0");
  position_hbox.set_border_width (10);
  position_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWindow::on_position_changed));

  add (vbox);
  scrolled_win.add (time_freq_view);
  show_all_children();

  navigator.signal_dhandle_changed.connect (sigc::mem_fun (*this, &MainWindow::on_dhandle_changed));
  navigator.signal_show_position_changed.connect (sigc::mem_fun (*this, &MainWindow::on_position_changed));

  spectrum_window.set_spectrum_model (time_freq_view);
}

void
MainWindow::on_zoom_changed()
{
  time_freq_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
MainWindow::on_position_changed()
{
  int frames = time_freq_view.get_frames();
  int position = CLAMP (sm_round_positive (position_adjustment.get_value() * frames), 0, frames - 1);
  char buffer[1024];
  sprintf (buffer, "frame %d", position);
  position_label.set_text (buffer);
  if (navigator.get_show_position())
    time_freq_view.set_position (position);
  else
    time_freq_view.set_position (-1);
}

void
MainWindow::on_dhandle_changed()
{
  time_freq_view.load (navigator.get_dhandle(), "fn");
}

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  assert (argc == 2 || argc == 3);
  if (argc == 3)
    {
      if (string (argv[2]) == "perf")
        {
          const double clocks_per_sec = 2500.0 * 1000 * 1000;
          const unsigned int runs = 10;

          PixelArray image;
          Glib::RefPtr<Gdk::Pixbuf> zimage;
          double hzoom = 1.3, vzoom = 1.5;
          image.resize (1024, 1024);

          // warmup run:
          zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1);

          // timed runs:
          double start = gettime();
          for (unsigned int i = 0; i < runs; i++)
            zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1);
          double end = gettime();

          printf ("zoom_rect: %f clocks/pixel\n", clocks_per_sec * (end - start) / (300 * 300) / runs);
          printf ("zoom_rect: %f Mpixel/s\n", 1.0 / ((end - start) / (300 * 300) / runs) / 1000 / 1000);

          return 0;
        }
      else
        {
          assert (false);
        }
    }

  MainWindow window (argv[1]);

  Gtk::Main::run (window);
}
