// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
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
#include "smutils.hh"

#include <QAction>
#include <QMenuBar>
#include <QMenu>

using namespace SpectMorph;

using std::string;
using std::vector;

MainWidget::MainWidget (bool use_jack)
{
  samples = NULL;
  current_wave = NULL;

  if (use_jack)
    jack_player = new SimpleJackPlayer ("smsampleedit");
  else
    jack_player = NULL;

  QGridLayout *grid = new QGridLayout();

  sample_view = new SampleView();

  scroll_area = new QScrollArea();
  scroll_area->setWidgetResizable (true);
  scroll_area->setWidget (sample_view);
  grid->addWidget (scroll_area, 0, 0, 1, 3);

  zoom_controller = new ZoomController (this, 1, 5000, 10, 5000),
  zoom_controller->set_hscrollbar (scroll_area->horizontalScrollBar());
  for (int i = 0; i < 3; i++)
    {
      grid->addWidget (zoom_controller->hwidget (i), 1, i);
      grid->addWidget (zoom_controller->vwidget (i), 2, i);
    }
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
  connect (save_button, SIGNAL (clicked()), this, SLOT (on_save_clicked()));

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

  grid->addLayout (button_hbox, 3, 0, 1, 3);
  setLayout (grid);
}

MainWidget::~MainWidget()
{
  if (jack_player)
    {
      delete jack_player;
      jack_player = NULL;
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
  double new_decoder_volume = db_to_factor (new_volume);
  volume_label->setText (string_locale_printf ("%.1f dB", new_volume).c_str());

  if (jack_player)
    jack_player->set_volume (new_decoder_volume);
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
      wave.label = string_locale_printf ("%s (note %d)", wset.waves[i].path.c_str(), wset.waves[i].midi_note);
      waves.push_back (wave);
    }
  // create combobox entries
  for (size_t i = 0; i < waves.size(); i++)
    sample_combobox->addItem (waves[i].label.c_str());

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

void
MainWidget::clip (const string& export_pattern)
{
  for (vector<Wave>::iterator wi = waves.begin(); wi != waves.end(); wi++)
    {
      WavData wav_in;
      if (!wav_in.load_mono (wi->path))
        {
          fprintf (stderr, "loading file %s failed: %s\n", wi->path.c_str(), wav_in.error_blurb());
          exit (1);
        }
      vector<float> clipped_samples = get_clipped_samples (&*wi, &wav_in);

      string export_wav = string_printf (export_pattern.c_str(), wi->midi_note);

      WavData wav_data (clipped_samples, 1, samples->mix_freq(), wav_in.bit_depth());
      if (!wav_data.save (export_wav))
        {
          fprintf (stderr, "export to file %s failed: %s\n", export_wav.c_str(), wav_data.error_blurb());
          exit (1);
        }
    }
}

void
MainWidget::on_combo_changed()
{
  samples.reset();

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

  samples.reset (new WavData());
  if (samples->load_mono (filename))
    {
      audio.mix_freq = samples->mix_freq();
      audio.fundamental_freq = 440; /* doesn't matter */
      sample_view->load (samples.get(), &audio, &wi->markers);
    }
  else
    {
      fprintf (stderr, "Loading audio file %s failed: %s\n", filename.c_str(), samples->error_blurb());
      samples.reset();

      sample_view->load ((WavData *) 0, nullptr, nullptr); // FIXME: NOBSE: remove cast
    }

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
  time_label->setText (string_locale_printf ("Time: %02d:%02d:%03d ms", m, s, ms).c_str());
}

vector<float>
MainWidget::get_clipped_samples (Wave *wave, WavData *samples)
{
  vector<float> result;

  if (samples && wave)
    {
      result = samples->samples();

      bool  clip_end_valid;
      float clip_end = wave->markers.clip_end (clip_end_valid);
      if (clip_end_valid && clip_end >= 0)
        {
          int iclipend = sm_round_positive (clip_end * samples->mix_freq() / 1000.0);
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
          int iclipstart = sm_round_positive (clip_start * samples->mix_freq() / 1000.0);
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
  audio.original_samples = get_clipped_samples (current_wave, samples.get());

  if (jack_player)
    jack_player->play (&audio, true);
}

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

MainWindow::MainWindow (bool use_jack)
{
  main_widget = new MainWidget (use_jack);

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

  MainWindow main_window (mode != CLIP);

  if (mode != CLIP)
    main_window.show();

  main_window.load (wav_set, clip_markers);
  if (mode == CLIP)
    {
      main_window.clip (export_pattern);
      return 0;
    }
  return app.exec();
}
