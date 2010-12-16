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

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

class Index : public Gtk::Window
{
  WavSet            wset;

  string            smset_dir;
  Gtk::ComboBoxText smset_combobox;
  Gtk::VBox         index_vbox;
  Gtk::ToggleButton show_position_button;
  vector<float>     decoded_samples;

  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    ModelColumns()
    {
      add (col_note);
      add (col_channel);
      add (col_range);
      add (col_path);
      add (col_wave_nr);
    }
    Gtk::TreeModelColumn<int>           col_note;
    Gtk::TreeModelColumn<int>           col_channel;
    Gtk::TreeModelColumn<Glib::ustring> col_range;
    Gtk::TreeModelColumn<Glib::ustring> col_path;
    Gtk::TreeModelColumn<int>           col_wave_nr;
  };

  ModelColumns audio_chooser_cols;
  Glib::RefPtr<Gtk::ListStore>       ref_tree_model;
  Glib::RefPtr<Gtk::TreeSelection>   ref_tree_selection;
  Gtk::ScrolledWindow                tree_view_scrolled_window;
  Gtk::TreeView                      tree_view;

  Gtk::ToggleButton                  source_button;

  GslDataHandle                     *dhandle;

public:
  sigc::signal<void> signal_dhandle_changed;
  sigc::signal<void> signal_show_position_changed;

  Index (const string& filename);

  void on_combo_changed();
  void on_selection_changed();
  void on_show_position_changed();

  GslDataHandle *get_dhandle();
  bool           get_show_position();
};

Index::Index (const string& filename) :
  dhandle (NULL)
{
  printf ("loading index: %s\n", filename.c_str());
  MicroConf cfg (filename);

  while (cfg.next())
    {
      string str;

      if (cfg.command ("smset", str))
        {
          smset_combobox.append_text (str);
        }
      else if (cfg.command ("smset_dir", str))
        {
          smset_dir = str;
        }
      else
        {
          cfg.die_if_unknown();
        }
    }

  set_title ("Instrument Index");
  set_border_width (10);
  set_default_size (300, 600);
  index_vbox.set_spacing (10);
  index_vbox.pack_start (smset_combobox, Gtk::PACK_SHRINK);
  ref_tree_model = Gtk::ListStore::create (audio_chooser_cols);
  tree_view.set_model (ref_tree_model);
  tree_view_scrolled_window.add (tree_view);
  index_vbox.pack_start (tree_view_scrolled_window);

  tree_view.append_column ("Note", audio_chooser_cols.col_note);
  tree_view.append_column ("Ch", audio_chooser_cols.col_channel);
  tree_view.append_column ("Range", audio_chooser_cols.col_range);
  tree_view.append_column ("Path", audio_chooser_cols.col_path);

  ref_tree_selection = tree_view.get_selection();
  ref_tree_selection->signal_changed().connect (sigc::mem_fun (*this, &Index::on_selection_changed));

  source_button.set_label ("Source/Analysis");
  index_vbox.pack_start (source_button, Gtk::PACK_SHRINK);
  source_button.signal_toggled().connect (sigc::mem_fun (*this, &Index::on_selection_changed));
  add (index_vbox);

  show_position_button.set_label ("Show Position");
  index_vbox.pack_start (show_position_button, Gtk::PACK_SHRINK);
  show_position_button.signal_toggled().connect (sigc::mem_fun (*this, &Index::on_show_position_changed));

  show();
  show_all_children();
  smset_combobox.signal_changed().connect (sigc::mem_fun (*this, &Index::on_combo_changed));
}

void
Index::on_selection_changed()
{
  Gtk::TreeModel::iterator iter = ref_tree_selection->get_selected();
  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;
      size_t i = row[audio_chooser_cols.col_wave_nr];
      assert (i < wset.waves.size());

      int channel = row[audio_chooser_cols.col_channel];

      Audio *audio = wset.waves[i].audio;
      assert (wset.waves[i].audio);

      if (source_button.get_active())
        {
          LiveDecoder decoder (&wset);
          decoder.retrigger (channel, audio->fundamental_freq, 127, audio->mix_freq);
          decoded_samples.resize (audio->sample_count);
          decoder.process (decoded_samples.size(), 0, 0, &decoded_samples[0]);
          dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, decoded_samples.size(), &decoded_samples[0], NULL);
        }
      else
        {
          dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, audio->original_samples.size(), &audio->original_samples[0], NULL);
        }
      signal_dhandle_changed();
    }
}

void
Index::on_combo_changed()
{
  std::string file = smset_dir + "/" + smset_combobox.get_active_text().c_str();
  printf ("loading %s...\n", file.c_str());
  BseErrorType error = wset.load (file);
  if (error)
    {
      fprintf (stderr, "sminspector: can't open input file: %s: %s\n", file.c_str(), bse_error_blurb (error));
      exit (1);
    }

  ref_tree_model->clear();
  for (size_t i = 0; i < wset.waves.size(); i++)
    {
      const WavSetWave& wave = wset.waves[i];

      Gtk::TreeModel::Row row = *(ref_tree_model->append());
      row[audio_chooser_cols.col_note] = wave.midi_note;
      row[audio_chooser_cols.col_channel] = wave.channel;
      row[audio_chooser_cols.col_range] = Birnet::string_printf ("%d..%d", wave.velocity_range_min, wave.velocity_range_max);
      row[audio_chooser_cols.col_path] = wave.path;
      row[audio_chooser_cols.col_wave_nr] = i;
    }
}

void
Index::on_show_position_changed()
{
  signal_show_position_changed();
}

GslDataHandle *
Index::get_dhandle()
{
  return dhandle;
}

bool
Index::get_show_position()
{
  return show_position_button.get_active();
}

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
  Index               index;
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
  index (filename)
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

  index.signal_dhandle_changed.connect (sigc::mem_fun (*this, &MainWindow::on_dhandle_changed));
  index.signal_show_position_changed.connect (sigc::mem_fun (*this, &MainWindow::on_position_changed));

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
  if (index.get_show_position())
    time_freq_view.set_position (position);
  else
    time_freq_view.set_position (-1);
}

void
MainWindow::on_dhandle_changed()
{
  time_freq_view.load (index.get_dhandle(), "fn");
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
