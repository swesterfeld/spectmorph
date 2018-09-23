// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "spectmorphglui.hh"

#include <jack/jack.h>
#include <jack/midiport.h>

#include <stdio.h>
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::max;
using std::vector;

class JackBackend
{
  double jack_mix_freq;
  jack_port_t *input_port;
  jack_port_t *output_port;
public:


  static int
  jack_process (jack_nframes_t nframes, void *arg)
  {
    JackBackend *instance = reinterpret_cast<JackBackend *> (arg);
    return instance->process (nframes);
  }

  int
  process (jack_nframes_t nframes)
  {
    float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);

    for (jack_nframes_t i = 0; i < nframes; i++)
      audio_out[i] = g_random_double_range (-0.1, 0.1);

    return 0;
  }

  JackBackend (jack_client_t *client)
  {
    jack_mix_freq = jack_get_sample_rate (client);

    jack_set_process_callback (client, jack_process, this);

    input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (jack_activate (client))
      {
        fprintf (stderr, "cannot activate client");
        exit (1);
      }
  }
};

// morph plan window size
namespace
{
  static const int win_width = 744;
  static const int win_height = 560;
};

class SampleWidget : public Widget
{
  void
  draw_grid (cairo_t *cr)
  {
    DrawUtils du (cr);

    du.set_color (Color (0.33, 0.33, 0.33));
    cairo_set_line_width (cr, 1);

    const double pad = 8;
    for (double y = pad; y < height - 4; y += pad)
      {
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width, y);
        cairo_stroke (cr);
      }
    for (double x = pad; x < width - 4; x += pad)
      {
        cairo_move_to (cr, x, 0);
        cairo_line_to (cr, x, height);
        cairo_stroke (cr);
      }
  }
  vector<float> samples;
public:
  SampleWidget (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    draw_grid (cr);
    /* redraw border to overdraw line endings */
    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color::null());

    //du.set_color (Color (0.4, 0.4, 1.0));
    du.set_color (Color (0.9, 0.1, 0.1));
    for (size_t i = 0; i < samples.size(); i++)
      {
        cairo_move_to (cr, double (i) * width / samples.size(), height / 2);
        cairo_line_to (cr, double (i) * width / samples.size(), height / 2 + (height * samples[i]));
        cairo_stroke (cr);
      }

    du.set_color (Color (1.0, 0.3, 0.3));
    cairo_move_to (cr, 0, height/2);
    cairo_line_to (cr, width, height/2);
    cairo_stroke (cr);
  }
  void
  set_samples (const vector<float>& samples)
  {
    this->samples = samples;
  }
};

class MainWindow : public Window
{
  SampleWidget *sample_widget;
  void
  load_sample (const string& filename)
  {
    if (filename != "")
      {
        WavData wav_data;
        if (wav_data.load_mono (filename))
          {
            printf ("%s %f %zd\n", filename.c_str(), wav_data.mix_freq(), wav_data.samples().size());
            sample_widget->set_samples (wav_data.samples());
          }
      }
  }
  ScrollView *sample_scroll_view;
  Label *vzoom_label;
public:
  void
  on_add_sample_clicked()
  {
    open_file_dialog ("Select Sample to load", "Wav Files", "*.wav", [=](string filename) {
      load_sample (filename);
    });
  }
  MainWindow() :
    Window ("SpectMorph - Instrument Editor", win_width, win_height)
  {
    FixedGrid grid;

    MenuBar *menu_bar = new MenuBar (this);

    Menu *file_menu = menu_bar->add_menu ("File");

    MenuItem *import_item = file_menu->add_item ("Add Sample...");
    connect (import_item->signal_clicked, this, &MainWindow::on_add_sample_clicked);

    grid.add_widget (menu_bar, 1, 1, 91, 3);

    sample_scroll_view = new ScrollView (this);
    grid.add_widget (sample_scroll_view, 1, 5, 91, 46);

    sample_widget = new SampleWidget (sample_scroll_view);

    grid.add_widget (sample_widget, 1, 1, 100, 42);
    sample_scroll_view->set_scroll_widget (sample_widget, true, false);

    load_sample ("/home/stefan/src/spectmorph-trumpet/samples/trumpet/trumpet-ff-c4.flac");
    grid.add_widget (new Label (this, "VZoom"), 1, 51, 10, 3);
    Slider *slider = new Slider (this, 0.0);
    grid.add_widget (slider, 8, 51, 30, 3);
    connect (slider->signal_value_changed, this, &MainWindow::on_update_hzoom);

    vzoom_label = new Label (this, "0");
    grid.add_widget (vzoom_label, 40, 51, 10, 3);

    // show complete wave
    on_update_hzoom (0);
  }
  void
  on_update_hzoom (float value)
  {
    FixedGrid grid;
    double factor = pow (2, value * 10);
    grid.add_widget (sample_widget, 1, 1, 89 * factor, 42);
    sample_scroll_view->on_widget_size_changed();
    vzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  jack_client_t *client = jack_client_open ("sminstedit", JackNullOption, NULL);
  if (!client)
    {
      fprintf (stderr, "%s: unable to connect to jack server\n", argv[0]);
      exit (1);
    }

  JackBackend jack_backend (client);

  bool quit = false;

  MainWindow window;

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_for_event();
      window.process_events();
    }
  jack_deactivate (client);
}
