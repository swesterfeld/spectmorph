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
#include "smmicroconf.hh"
#include "smsampleedit.hh"

#include <QAction>
#include <QMenuBar>
#include <QMenu>

using namespace SpectMorph;

using std::string;
using std::vector;

GslDataHandle* dhandle_from_file (const string& filename);


MainWidget::MainWidget() :
  jack_player ("smsampleedit")
{
  samples = NULL;

  QVBoxLayout *vbox = new QVBoxLayout();

  sample_view = new SampleView();

  scroll_area = new QScrollArea();
  scroll_area->setWidgetResizable (true);
  scroll_area->setWidget (sample_view);
  vbox->addWidget (scroll_area);

  zoom_controller = new ZoomController (1, 5000, 10, 5000),
  vbox->addWidget (zoom_controller);
  connect (zoom_controller, SIGNAL (zoom_changed()), this, SLOT (on_zoom_changed()));

  QHBoxLayout *button_hbox = new QHBoxLayout();
  sample_combobox = new QComboBox();
  connect (sample_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_combo_changed()));

  time_label = new QLabel();
  volume_label = new QLabel();
  connect (sample_view, SIGNAL (mouse_time_changed (int)), this, SLOT (on_mouse_time_changed (int)));
  on_mouse_time_changed (0);

  QPushButton *play_button = new QPushButton ("Play");
  QPushButton *save_button = new QPushButton ("Save");
  connect (play_button, SIGNAL (clicked()), this, SLOT (on_play_clicked()));

  volume_slider = new QSlider (Qt::Horizontal, this);
  volume_slider->setRange (-96000, 24000);
  connect (volume_slider, SIGNAL (valueChanged(int)), this, SLOT (on_volume_changed(int)));
  on_volume_changed (0);

  edit_clip_start = new QPushButton ("Edit Clip Start");
  edit_clip_end = new QPushButton ("Edit Clip End");
  edit_clip_start->setCheckable (true);
  edit_clip_end->setCheckable (true);
  connect (edit_clip_start, SIGNAL (clicked()), this, SLOT (on_edit_marker_changed()));
  connect (edit_clip_end, SIGNAL (clicked()), this, SLOT (on_edit_marker_changed()));

  button_hbox->addWidget (time_label);
  button_hbox->addWidget (sample_combobox);
  button_hbox->addWidget (play_button);
  button_hbox->addWidget (save_button);
  button_hbox->addWidget (new QLabel ("Volume"));
  button_hbox->addWidget (volume_slider);
  button_hbox->addWidget (volume_label);
  button_hbox->addWidget (edit_clip_start);
  button_hbox->addWidget (edit_clip_end);

  vbox->addLayout (button_hbox);
  setLayout (vbox);
}

#if 0
MainWidget::MainWidget() :
  volume_scale (-96, 20, 0.01),
  zoom_controller (1, 5000, 10, 5000),
  jack_player ("smsampleedit")
{
  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("SampleMenu", "Sample"));
  ref_action_group->add (Gtk::Action::create ("SampleNext", "Next Sample"), Gtk::AccelKey ('n', Gdk::ModifierType (0)),
                         sigc::mem_fun (*this, &MainWidget::on_next_sample));

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
  samples = NULL;

  scrolled_win.add (sample_view);

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  vbox.pack_start (button_hbox, Gtk::PACK_SHRINK);

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MainWidget::on_zoom_changed));
  sample_view.signal_mouse_time_changed.connect (sigc::mem_fun (*this, &MainWidget::on_mouse_time_changed));

  edit_clip_start.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &MainWidget::on_edit_marker_changed),
                                            SampleView::MARKER_CLIP_START));
  edit_clip_end.signal_toggled().connect (sigc::bind (sigc::mem_fun (*this, &MainWidget::on_edit_marker_changed),
                                          SampleView::MARKER_CLIP_END));
  play_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWidget::on_play_clicked));
  save_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWidget::on_save_clicked));

  sample_view.signal_resized.connect (sigc::mem_fun (*this, &MainWidget::on_resized));

  sample_combobox.signal_changed().connect (sigc::mem_fun (*this, &MainWidget::on_combo_changed));

  on_mouse_time_changed (0); // init label

  play_button.set_label ("Play");
  save_button.set_label ("Save");
  edit_clip_start.set_label ("Edit Clip Start");
  edit_clip_end.set_label ("Edit Clip End");
  button_hbox.pack_start (time_label, Gtk::PACK_SHRINK);
  button_hbox.pack_start (sample_combobox);
  button_hbox.pack_start (play_button);
  button_hbox.pack_start (save_button);
  button_hbox.pack_start (volume_hbox);
  button_hbox.pack_start (edit_clip_start);
  button_hbox.pack_start (edit_clip_end);

  volume_hbox.pack_start (volume_label, Gtk::PACK_SHRINK);
  volume_hbox.pack_start (volume_scale);
  volume_hbox.pack_start (volume_value_label, Gtk::PACK_SHRINK);
  volume_label.set_label ("Volume");
  volume_scale.set_draw_value (false);
  volume_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWidget::on_volume_changed));
  volume_scale.set_value (0);  // calls on_volume_changed()

  add (vbox);
  show_all_children();
}
#endif

MainWidget::~MainWidget()
{
  if (samples)
    {
      delete samples;
      samples = NULL;
    }
}

void
MainWidget::on_next_sample()
{
  string label = sample_combobox->currentText().toLatin1().data();
  vector<Wave>::iterator wi = waves.begin();
  while (wi != waves.end())
    {
      if (wi->label == label)
        break;
      else
        wi++;
    }
  if (wi == waves.end()) // should not happen
    return;
  wi++; // next sample
  if (wi == waves.end())
    {
      // no next sample
      return;
    }
  sample_combobox->setCurrentText (wi->label.c_str());
}

void
MainWidget::on_volume_changed (int new_volume_int)
{
  double new_volume = new_volume_int / 1000.0;
  double new_decoder_volume = bse_db_to_factor (new_volume);
  volume_label->setText (Birnet::string_printf ("%.1f dB", new_volume).c_str());

  jack_player.set_volume (new_decoder_volume);
}

void
MainWidget::on_edit_marker_changed()
{
  SampleView::EditMarkerType marker_type;

  QPushButton *btn = qobject_cast<QPushButton *> (sender());
  if (btn == edit_clip_start)
    marker_type = SampleView::MARKER_CLIP_START;
  else if (btn == edit_clip_end)
    marker_type = SampleView::MARKER_CLIP_END;
  else
    g_assert_not_reached();

  if (sample_view->edit_marker_type() == marker_type)  // we're selected already -> turn it off
    marker_type = SampleView::MARKER_NONE;

  sample_view->set_edit_marker_type (marker_type);

  edit_clip_start->setChecked (marker_type == SampleView::MARKER_CLIP_START);
  edit_clip_end->setChecked (marker_type == SampleView::MARKER_CLIP_END);
}

#if 0
void
MainWidget::on_resized (int old_width, int new_width)
{
  if (old_width > 0 && new_width > 0)
    {
      Gtk::Viewport *view_port = dynamic_cast<Gtk::Viewport*> (scrolled_win.get_child());
      Gtk::Adjustment *hadj = scrolled_win.get_hadjustment();

      const int w_2 = view_port->get_width() / 2;

      hadj->set_value ((hadj->get_value() + w_2) / old_width * new_width - w_2);
    }
}
#endif

void
MainWidget::load (const string& filename, const string& clip_markers)
{
  WavSet wset;
  wset.load (filename);
  for (size_t i = 0; i < wset.waves.size(); i++)
    {
      Wave wave;
      wave.path = wset.waves[i].path;
      wave.midi_note = wset.waves[i].midi_note;
      wave.label = Birnet::string_printf ("%s (note %d)", wset.waves[i].path.c_str(), wset.waves[i].midi_note);
      sample_combobox->addItem (wave.label.c_str());
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
  on_combo_changed();
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
MainWidget::clip (const string& export_pattern)
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
MainWidget::on_combo_changed()
{
  if (samples)
    {
      delete samples;
      samples = NULL;
    }
  string label = sample_combobox->currentText().toLatin1().data();
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
  sample_view->load (dhandle, &audio, &wi->markers);
  gsl_data_handle_close (dhandle);

  current_wave = &(*wi);
}

void
MainWidget::on_zoom_changed()
{
  sample_view->set_zoom (zoom_controller->get_hzoom(), zoom_controller->get_vzoom());
}

void
MainWidget::on_mouse_time_changed (int time)
{
  int ms = time % 1000;
  time /= 1000;
  int s = time % 60;
  time /= 60;
  int m = time;
  time_label->setText (Birnet::string_printf ("Time: %02d:%02d:%03d ms", m, s, ms).c_str());
}

vector<float>
MainWidget::get_clipped_samples (Wave *wave, WavLoader *samples)
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
MainWidget::on_play_clicked()
{
  audio.original_samples = get_clipped_samples (current_wave, samples);

  jack_player.play (&audio, true);
}

#if 0
static string
double_to_string (double value)
{
  gchar numbuf[G_ASCII_DTOSTR_BUF_SIZE + 1] = "";
  g_ascii_formatd (numbuf, G_ASCII_DTOSTR_BUF_SIZE, "%.9g", value);
  return numbuf;
}

void
MainWidget::on_save_clicked()
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
#endif

MainWindow::MainWindow()
{
  main_widget = new MainWidget();

  /* actions ... */
  QAction *next_action = new QAction ("Next Sample", this);
  next_action->setShortcut (QString ("n"));
  connect (next_action, SIGNAL (triggered()), main_widget, SLOT (on_next_sample()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *sample_menu = menu_bar->addMenu ("&Sample");
  sample_menu->addAction (next_action);

  setCentralWidget (main_widget);

  resize (800, 600);
}

void
MainWindow::load (const string& filename, const string& clip_markers)
{
  main_widget->load (filename, clip_markers);
}

void
MainWindow::clip (const string& export_pattern)
{
  main_widget->clip (export_pattern);
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

  QApplication app (argc, argv);

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
  main_window.show();
  main_window.load (wav_set, clip_markers);
  if (mode == CLIP)
    {
      main_window.clip (export_pattern);
      return 0;
    }
  return app.exec();
}
