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
using std::map;

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

enum MarkerType {
  MARKER_NONE = 0,
  MARKER_LOOP_START,
  MARKER_LOOP_END,
  MARKER_CLIP_START,
  MARKER_CLIP_END
};

class Markers
{
  map<MarkerType, double> pos_map;
public:
  void
  clear()
  {
    pos_map.clear();
  }
  void
  set (MarkerType marker_type, double value)
  {
    pos_map[marker_type] = value;
  }
  double
  get (MarkerType marker_type)
  {
    auto it = pos_map.find (marker_type);
    if (it != pos_map.end())
      return it->second;
    return -1;
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
  draw_grid (const DrawEvent& devent)
  {
    cairo_t *cr = devent.cr;
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
        if (x >= devent.rect.x() && x <= devent.rect.x() + devent.rect.width())
          {
            cairo_move_to (cr, x, 0);
            cairo_line_to (cr, x, height);
            cairo_stroke (cr);
          }
      }
  }
  vector<float> samples;
  double        mix_freq;
  double        vzoom = 1;
  Markers      *m_markers = nullptr;
public:
  SampleWidget (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    draw_grid (devent);

    /* redraw border to overdraw line endings */
    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color::null());

    const double clip_start_x = mix_freq * m_markers->get (MARKER_CLIP_START) / 1000. * width / samples.size();
    const double clip_end_x = mix_freq * m_markers->get (MARKER_CLIP_END) / 1000. * width / samples.size();
    const double loop_start_x = mix_freq * m_markers->get (MARKER_LOOP_START) / 1000. * width / samples.size();
    const double loop_end_x = mix_freq * m_markers->get (MARKER_LOOP_END) / 1000. * width / samples.size();

    /* modify background widget in loop region */
    cairo_rectangle (cr, loop_start_x, 0, loop_end_x - loop_start_x, height);
    cairo_set_source_rgba (cr, 0.5, 0.5, 1.0, 0.25);
    cairo_fill (cr);

    //du.set_color (Color (0.4, 0.4, 1.0));
    du.set_color (Color (0.9, 0.1, 0.1));
    for (int pass = 0; pass < 2; pass++)
      {
        int last_x_pixel = -1;
        float max_s = 0;
        float min_s = 0;
        cairo_move_to (cr, 0, height / 2);
        for (size_t i = 0; i < samples.size(); i++)
          {
            double dx = double (i) * width / samples.size();

            if (dx >= devent.rect.x() && dx <= devent.rect.x() + devent.rect.width())
              {
                int x_pixel = dx;
                max_s = std::max (samples[i], max_s);
                min_s = std::min (samples[i], min_s);
                if (x_pixel != last_x_pixel)
                  {
                    if (pass == 0)
                      cairo_line_to (cr, last_x_pixel, height / 2 + min_s * height / 2 * vzoom);
                    else
                      cairo_line_to (cr, last_x_pixel, height / 2 + max_s * height / 2 * vzoom);

                    last_x_pixel = x_pixel;
                    max_s = 0;
                    min_s = 0;
                  }
              }
          }
        cairo_line_to (cr, last_x_pixel, height / 2);
        cairo_close_path (cr);
        cairo_set_line_width (cr, 1);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
      }

    /* darken widget before and after clip region */
    cairo_rectangle (cr, 0, 0, clip_start_x, height);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.25);
    cairo_fill (cr);
    cairo_rectangle (cr, clip_end_x, 0, width - clip_end_x, height);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.25);
    cairo_fill (cr);

    du.set_color (Color (1.0, 0.3, 0.3));
    cairo_move_to (cr, 0, height/2);
    cairo_line_to (cr, width, height/2);
    cairo_stroke (cr);

    /* markers */
    for (int m = MARKER_LOOP_START; m <= MARKER_CLIP_END; m++)
      {
        MarkerType marker = static_cast<MarkerType> (m);
        double marker_x = mix_freq * m_markers->get (marker) / 1000. * width / samples.size();

        if (m == MARKER_LOOP_START)
          {
            cairo_rectangle (cr, marker_x, 0, 10, 10);
            du.set_color (Color (1.0, 0.3, 0));
          }
        else if (m == MARKER_LOOP_END)
          {
            cairo_rectangle (cr, marker_x - 10, 0, 10, 10);
            du.set_color (Color (1.0, 0.3, 0));
          }
        else if (m == MARKER_CLIP_START)
          {
            cairo_rectangle (cr, marker_x, height - 10, 10, 10);
            du.set_color (Color (0.8, 0, 0));
          }
        else if (m == MARKER_CLIP_END)
          {
            cairo_rectangle (cr, marker_x - 10, height - 10, 10, 10);
            du.set_color (Color (0.8, 0, 0));
          }
        cairo_fill (cr);
        cairo_move_to (cr, marker_x, 0);
        cairo_line_to (cr, marker_x, height);
        cairo_stroke (cr);
      }
  }
  void
  set_samples (const vector<float>& samples, double mix_freq)
  {
    this->samples = samples;
    this->mix_freq = mix_freq;
    update();
  }
  void
  set_vzoom (double factor)
  {
    vzoom = factor;
    update();
  }
  void
  set_markers (Markers *markers)
  {
    m_markers = markers;
    update();
  }
};

class MainWindow : public Window
{
  SampleWidget *sample_widget;
  Markers markers;
  void
  load_sample (const string& filename)
  {
    if (filename != "")
      {
        WavData wav_data;
        if (wav_data.load_mono (filename))
          {
            printf ("%s %f %zd\n", filename.c_str(), wav_data.mix_freq(), wav_data.samples().size());
            sample_widget->set_samples (wav_data.samples(), wav_data.mix_freq());

            markers.clear();
            markers.set (MARKER_CLIP_START, 0.2 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
            markers.set (MARKER_CLIP_END, 0.9 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
            markers.set (MARKER_LOOP_START, 0.4 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
            markers.set (MARKER_LOOP_END, 0.6 * 1000.0 * wav_data.samples().size() / wav_data.mix_freq());
            sample_widget->set_markers (&markers);
          }
      }
  }
  ScrollView *sample_scroll_view;
  Label *hzoom_label;
  Label *vzoom_label;
public:
  void
  on_add_sample_clicked()
  {
    open_file_dialog ("Select Sample to load", "Wav Files", "*.wav", [=](string filename) {
      load_sample (filename);
    });
  }
  MainWindow (const string& test_sample) :
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
    sample_scroll_view->set_scroll_widget (sample_widget, true, false, /* center_zoom */ true);

    load_sample (test_sample);

    /*----- hzoom -----*/
    grid.add_widget (new Label (this, "HZoom"), 1, 51, 10, 3);
    Slider *hzoom_slider = new Slider (this, 0.0);
    grid.add_widget (hzoom_slider, 8, 51, 30, 3);
    connect (hzoom_slider->signal_value_changed, this, &MainWindow::on_update_hzoom);

    hzoom_label = new Label (this, "0");
    grid.add_widget (hzoom_label, 40, 51, 10, 3);

    /*----- vzoom -----*/
    grid.add_widget (new Label (this, "VZoom"), 1, 54, 10, 3);
    Slider *vzoom_slider = new Slider (this, 0.0);
    grid.add_widget (vzoom_slider, 8, 54, 30, 3);
    connect (vzoom_slider->signal_value_changed, this, &MainWindow::on_update_vzoom);

    vzoom_label = new Label (this, "0");
    grid.add_widget (vzoom_label, 40, 54, 10, 3);

    // show complete wave
    on_update_hzoom (0);

    on_update_vzoom (0);
  }
  void
  on_update_hzoom (float value)
  {
    FixedGrid grid;
    double factor = pow (2, value * 10);
    grid.add_widget (sample_widget, 1, 1, 89 * factor, 42);
    sample_scroll_view->on_widget_size_changed();
    hzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
  }
  void
  on_update_vzoom (float value)
  {
    FixedGrid grid;
    double factor = pow (10, value);
    sample_widget->set_vzoom (factor);
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

  string fn = (argc > 1) ? argv[1] : "/home/stefan/src/spectmorph-trumpet/samples/trumpet/trumpet-ff-c4.flac";
  MainWindow window (fn);

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_for_event();
      window.process_events();
    }
  jack_deactivate (client);
}
