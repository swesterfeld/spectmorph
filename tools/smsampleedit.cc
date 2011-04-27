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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <vector>
#include <string>
#include <iostream>

#include "smmain.hh"
#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include "smsimplejackplayer.hh"
#include "smwavloader.hh"
#include "smmicroconf.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

GslDataHandle* dhandle_from_file (const string& filename);

class SampleEditMarkers : public SampleView::Markers
{
public:
  vector<float> positions;
  vector<bool>  m_valid;

  SampleEditMarkers() :
    positions (2),
    m_valid (positions.size())
  {
  }
  size_t
  count()
  {
    return positions.size();
  }
  bool
  valid (size_t m)
  {
    return m_valid[m];
  }
  float
  position (size_t m)
  {
    g_return_val_if_fail (m >= 0 && m < positions.size(), 0.0);

    return positions[m];
  }
  void
  set_position (size_t m, float new_pos)
  {
    g_return_if_fail (m >= 0 && m < positions.size());

    positions[m] = new_pos;
    m_valid[m] = true;
  }
  SampleView::EditMarkerType
  type (size_t m)
  {
    if (m == 0)
      return SampleView::MARKER_CLIP_START;
    else if (m == 1)
      return SampleView::MARKER_CLIP_END;
    else
      return SampleView::MARKER_NONE;
  }
  void
  clear (size_t m)
  {
    g_return_if_fail (m >= 0 && m < positions.size());

    m_valid[m] = false;
  }
  float
  clip_start (bool& v)
  {
    v = m_valid[0];
    return positions[0];
  }
  float
  clip_end (bool& v)
  {
    v = m_valid[1];
    return positions[1];
  }
  void
  set_clip_start (float pos)
  {
    m_valid[0] = true;
    positions[0] = pos;
  }
  void
  set_clip_end (float pos)
  {
    m_valid[1] = true;
    positions[1] = pos;
  }
};

namespace {
struct Wave
{
  string              path;
  string              label;
  int                 midi_note;
  SampleEditMarkers   markers;
};
}

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  Gtk::HBox           button_hbox;
  Gtk::VBox           vbox;
  SampleView          sample_view;
  Audio               audio;
  ZoomController      zoom_controller;
  Gtk::Button         play_button;
  Gtk::Button         save_button;
  Gtk::ToggleButton   edit_clip_start;
  Gtk::ToggleButton   edit_clip_end;
  Gtk::Label          time_label;
  bool                in_update_buttons;
  SimpleJackPlayer    jack_player;
  WavLoader          *samples;
  Gtk::ComboBoxText   sample_combobox;
  vector<Wave>        waves;
  Wave               *current_wave;
  string              marker_filename;
  Glib::RefPtr<Gtk::UIManager>    ref_ui_manager;
  Glib::RefPtr<Gtk::ActionGroup>  ref_action_group;

  vector<float> get_clipped_samples (Wave *wave, WavLoader *samples);
public:
  MainWindow();

  void on_edit_marker_changed (SampleView::EditMarkerType marker_type);
  void on_play_clicked();
  void on_save_clicked();
  void on_zoom_changed();
  void on_next_sample();
  void on_combo_changed();
  void on_mouse_time_changed (int time);
  void load (const string& filename, const string& clip_markers);
  void clip (const string& export_pattern);
  void on_resized (int old_width, int new_width);
};

MainWindow::MainWindow() :
  zoom_controller (1, 5000, 10, 5000),
  jack_player ("smsampleedit")
{
  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("SampleMenu", "Sample"));
  ref_action_group->add (Gtk::Action::create ("SampleNext", "Next Sample"), Gtk::AccelKey ('n', Gdk::ModifierType (0)),
                         sigc::mem_fun (*this, &MainWindow::on_next_sample));

  ref_ui_manager = Gtk::UIManager::create();
  ref_ui_manager-> insert_action_group (ref_action_group);
  add_accel_group (ref_ui_manager->get_accel_group());

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='SampleMenu'>"
    "      <menuitem action='SampleNext' />"
    "    </menu>"
    "  </menubar>"
    "</ui>";
  try
    {
      ref_ui_manager->add_ui_from_string (ui_info);
    }
  catch (const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " << ex.what();
    }

  Gtk::Widget *menu_bar = ref_ui_manager->get_widget ("/MenuBar");
  if (menu_bar)
    vbox.pack_start (*menu_bar, Gtk::PACK_SHRINK);

  in_update_buttons = false;
  current_wave = NULL;
  scrolled_win.add (sample_view);

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  vbox.pack_start (button_hbox, Gtk::PACK_SHRINK);

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MainWindow::on_zoom_changed));
  sample_view.signal_mouse_time_changed.connect (sigc::mem_fun (*this, &MainWindow::on_mouse_time_changed));

  edit_clip_start.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &MainWindow::on_edit_marker_changed),
                                            SampleView::MARKER_CLIP_START));
  edit_clip_end.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &MainWindow::on_edit_marker_changed),
                                          SampleView::MARKER_CLIP_END));
  play_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWindow::on_play_clicked));
  save_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWindow::on_save_clicked));

  sample_view.signal_resized.connect (sigc::mem_fun (*this, &MainWindow::on_resized));

  sample_combobox.signal_changed().connect (sigc::mem_fun (*this, &MainWindow::on_combo_changed));

  on_mouse_time_changed (0); // init label

  play_button.set_label ("Play");
  save_button.set_label ("Save");
  edit_clip_start.set_label ("Edit Clip Start");
  edit_clip_end.set_label ("Edit Clip End");
  button_hbox.pack_start (time_label);
  button_hbox.pack_start (sample_combobox);
  button_hbox.pack_start (play_button);
  button_hbox.pack_start (save_button);
  button_hbox.pack_start (edit_clip_start);
  button_hbox.pack_start (edit_clip_end);
  add (vbox);
  show_all_children();

  jack_player.set_volume (0.125);
}

void
MainWindow::on_next_sample()
{
  printf ("FIXME: implement on_next_sample();\n");
}

void
MainWindow::on_edit_marker_changed (SampleView::EditMarkerType marker_type)
{
  if (in_update_buttons)
    return;

  if (sample_view.edit_marker_type() == marker_type)  // we're selected already -> turn it off
    marker_type = SampleView::MARKER_NONE;

  sample_view.set_edit_marker_type (marker_type);

  in_update_buttons = true;
  edit_clip_start.set_active (marker_type == SampleView::MARKER_CLIP_START);
  edit_clip_end.set_active (marker_type == SampleView::MARKER_CLIP_END);
  in_update_buttons = false;
}

void
MainWindow::on_resized (int old_width, int new_width)
{
  if (old_width > 0 && new_width > 0)
    {
      Gtk::Viewport *view_port = dynamic_cast<Gtk::Viewport*> (scrolled_win.get_child());
      Gtk::Adjustment *hadj = scrolled_win.get_hadjustment();

      const int w_2 = view_port->get_width() / 2;

      hadj->set_value ((hadj->get_value() + w_2) / old_width * new_width - w_2);
    }
}


void
MainWindow::load (const string& filename, const string& clip_markers)
{
  WavSet wset;
  wset.load (filename);
  for (size_t i = 0; i < wset.waves.size(); i++)
    {
      Wave wave;
      wave.path = wset.waves[i].path;
      wave.midi_note = wset.waves[i].midi_note;
      wave.label = Birnet::string_printf ("%s (note %d)", wset.waves[i].path.c_str(), wset.waves[i].midi_note);
      sample_combobox.append_text (wave.label);
      waves.push_back (wave);
    }
  printf ("loaded %zd waves.\n", wset.waves.size());
  marker_filename = clip_markers;
  MicroConf cfg (marker_filename.c_str());
  if (cfg.open_ok())
    {
      cfg.set_number_format (MicroConf::NO_I18N);

      while (cfg.next())
        {
          string marker_type;
          int    midi_note;
          double marker_pos;

          if (cfg.command ("set-marker", marker_type, midi_note, marker_pos))
            {
              vector<Wave>::iterator wi = waves.begin();

              while (wi != waves.end())
                {
                  if (wi->midi_note == midi_note)
                    break;
                  else
                    wi++;
                }
              if (wi == waves.end())
                {
                  g_printerr ("NOTE %d not found\n", midi_note);
                }
              else
                {
                  if (marker_type == "clip-start")
                    {
                      wi->markers.set_clip_start (marker_pos);
                    }
                  else if (marker_type == "clip-end")
                    {
                      wi->markers.set_clip_end (marker_pos);
                    }
                  else
                    {
                      g_printerr ("MARKER-TYPE %s not supported\n", marker_type.c_str());
                    }
                }
            }
          else
            {
              cfg.die_if_unknown();
            }
        }
    }
}

static void
dump_wav (string filename, const vector<float>& sample, double mix_freq, int n_channels)
{
  GslDataHandle *out_dhandle = gsl_data_handle_new_mem (n_channels, 32, mix_freq, 44100 / 16 * 2048, sample.size(), &sample[0], NULL);
  BseErrorType error = gsl_data_handle_open (out_dhandle);
  if (error)
    {
      fprintf (stderr, "can not open mem dhandle for exporting wave file\n");
      exit (1);
    }

  int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    {
      BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }
  int xerrno = gsl_data_handle_dump_wav (out_dhandle, fd, 16, out_dhandle->setup.n_channels, (guint) out_dhandle->setup.mix_freq);
  if (xerrno)
    {
      BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }
}


void
MainWindow::clip (const string& export_pattern)
{
  for (vector<Wave>::iterator wi = waves.begin(); wi != waves.end(); wi++)
    {
      WavLoader *samples = WavLoader::load (wi->path.c_str());
      vector<float> clipped_samples = get_clipped_samples (&*wi, samples);

      string export_wav = Birnet::string_printf (export_pattern.c_str(), wi->midi_note);
      dump_wav (export_wav, clipped_samples, samples->mix_freq(), 1);

      delete samples;
    }
}

void
MainWindow::on_combo_changed()
{
  string label = sample_combobox.get_active_text().c_str();
  vector<Wave>::iterator wi = waves.begin();
  while (wi != waves.end())
    {
      if (wi->label == label)
        break;
      else
        wi++;
    }
  if (wi == waves.end()) // should not happen
    {
      g_warning ("Wave for %s not found.\n", label.c_str());
      current_wave = NULL;
      return;
    }

  string path = wi->path;
  string sample_dir = "."; // FIXME
  string filename = sample_dir + "/" + path;

  if (!path.empty() && path[0] == '/') // absolute path
    filename = path;

  samples = WavLoader::load (filename);

  GslDataHandle *dhandle = dhandle_from_file (filename);
  gsl_data_handle_open (dhandle);
  audio.mix_freq = gsl_data_handle_mix_freq (dhandle);
  audio.fundamental_freq = 440; /* doesn't matter */
  sample_view.load (dhandle, &audio, &wi->markers);
  gsl_data_handle_close (dhandle);

  current_wave = &(*wi);
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

vector<float>
MainWindow::get_clipped_samples (Wave *wave, WavLoader *samples)
{
  vector<float> result;

  if (samples && wave)
    {
      result = samples->samples();

      bool  clip_end_valid;
      float clip_end = wave->markers.clip_end (clip_end_valid);
      if (clip_end_valid && clip_end >= 0)
        {
          int iclipend = clip_end * samples->mix_freq() / 1000.0;
          if (iclipend >= 0 && iclipend < int (result.size()))
            {
              vector<float>::iterator si = result.begin();

              result.erase (si + iclipend, result.end());
            }
        }

      bool  clip_start_valid;
      float clip_start = wave->markers.clip_start (clip_start_valid);
      if (clip_start_valid && clip_start >= 0)
        {
          int iclipstart = clip_start * samples->mix_freq() / 1000.0;
          if (iclipstart >= 0 && iclipstart < int (result.size()))
            {
              vector<float>::iterator si = result.begin();

              result.erase (si, si + iclipstart);
            }
        }
    }
  return result;
}

void
MainWindow::on_play_clicked()
{
  audio.original_samples = get_clipped_samples (current_wave, samples);

  jack_player.play (&audio, true);
}

static string
double_to_string (double value)
{
  gchar numbuf[G_ASCII_DTOSTR_BUF_SIZE + 1] = "";
  g_ascii_formatd (numbuf, G_ASCII_DTOSTR_BUF_SIZE, "%.9g", value);
  return numbuf;
}

void
MainWindow::on_save_clicked()
{
  FILE *file = fopen (marker_filename.c_str(), "w");
  g_return_if_fail (file != NULL);

  for (vector<Wave>::iterator wi = waves.begin(); wi != waves.end(); wi++)
    {
      bool  clip_start_valid;
      float clip_start = wi->markers.clip_start (clip_start_valid);

      if (clip_start_valid)
        fprintf (file, "set-marker clip-start %d %s\n", wi->midi_note, double_to_string (clip_start).c_str());

      bool  clip_end_valid;
      float clip_end = wi->markers.clip_end (clip_end_valid);
      if (clip_end_valid)
        fprintf (file, "set-marker clip-end %d %s\n", wi->midi_note, double_to_string (clip_end).c_str());
    }
  fclose (file);
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

  enum { EDIT, CLIP } mode;
  string wav_set, clip_markers, export_pattern;

  if (argc == 5 && strcmp (argv[1], "clip") == 0)
    {
      mode = CLIP;
      wav_set = argv[2];
      clip_markers = argv[3];
      export_pattern = argv[4];
    }
  else if (argc == 3)
    {
      mode = EDIT;
      wav_set = argv[1];
      clip_markers = argv[2];
    }
  else
    {
      printf ("usage: %s <wavset> <clip-markers>\n", argv[0]);
      printf ("usage: %s clip <wavset> <clip-markers> <export-pattern>\n", argv[0]);
      exit (1);
    }

  MainWindow main_window;
  main_window.load (wav_set, clip_markers);
  if (mode == CLIP)
    {
      main_window.clip (export_pattern);
      return 0;
    }
  Gtk::Main::run (main_window);

  return 0;
}
