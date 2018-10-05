// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "spectmorphglui.hh"
#include "smpugixml.hh"
#include "sminstrument.hh"

#include <jack/jack.h>
#include <jack/midiport.h>

#include <stdio.h>
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::max;
using std::vector;
using std::map;

using pugi::xml_document;
using pugi::xml_node;

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
  double        vzoom = 1;
  Sample       *m_sample = nullptr;
  MarkerType    selected_marker = MARKER_NONE;
  bool          mouse_down = false;
  map<MarkerType, Rect> marker_rect;
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

    if (!m_sample)
      return;

    const double length_ms = m_sample->wav_data.samples().size() / m_sample->wav_data.mix_freq() * 1000;
    const double clip_start_x = m_sample->get_marker (MARKER_CLIP_START) / length_ms * width;
    const double clip_end_x = m_sample->get_marker (MARKER_CLIP_END) / length_ms * width;
    const double loop_start_x = m_sample->get_marker (MARKER_LOOP_START) / length_ms * width;
    const double loop_end_x = m_sample->get_marker (MARKER_LOOP_END) / length_ms * width;
    const vector<float>& samples = m_sample->wav_data.samples();

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

    /* lighten loop region */
    cairo_rectangle (cr, loop_start_x, 0, loop_end_x - loop_start_x, height);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.25);
    cairo_fill (cr);

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
        double marker_x = m_sample->get_marker (marker) / length_ms * width;

        Rect  rect;
        Color color;
        if (m == MARKER_LOOP_START)
          {
            rect = Rect (marker_x, 0, 10, 10);
            color = Color (0.7, 0.7, 1);
          }
        else if (m == MARKER_LOOP_END)
          {
            rect = Rect (marker_x - 10, 0, 10, 10);
            color = Color (0.7, 0.7, 1);
          }
        else if (m == MARKER_CLIP_START)
          {
            rect = Rect (marker_x, height - 10, 10, 10);
            color = Color (0.4, 0.4, 1);
          }
        else if (m == MARKER_CLIP_END)
          {
            rect = Rect (marker_x - 10, height - 10, 10, 10);
            color = Color (0.4, 0.4, 1);
          }
        marker_rect[marker] = rect;

        if (marker == selected_marker)
          color = color.lighter (175);
        du.set_color (color);

        cairo_rectangle (cr, rect.x(), rect.y(), rect.width(), rect.height());
        cairo_fill (cr);
        cairo_move_to (cr, marker_x, 0);
        cairo_line_to (cr, marker_x, height);
        cairo_stroke (cr);
      }
  }
  MarkerType
  find_marker_xy (double x, double y)
  {
    for (int m = MARKER_LOOP_START; m <= MARKER_CLIP_END; m++)
      {
        MarkerType marker = MarkerType (m);

        if (marker_rect[marker].contains (x, y))
          return marker;
      }
    return MARKER_NONE;
  }
  void
  get_order (MarkerType marker, vector<MarkerType>& left, vector<MarkerType>& right)
  {
    vector<MarkerType> left_to_right { MARKER_CLIP_START, MARKER_LOOP_START, MARKER_LOOP_END, MARKER_CLIP_END };

    vector<MarkerType>::iterator it = find (left_to_right.begin(), left_to_right.end(), marker);
    size_t pos = it - left_to_right.begin();

    left.clear();
    right.clear();

    for (size_t i = 0; i < left_to_right.size(); i++)
      {
        const MarkerType lr_marker = left_to_right[i];
        if (i < pos)
          left.push_back (lr_marker);
        if (i > pos)
          right.push_back (lr_marker);
      }
  }
  void
  motion (double x, double y) override
  {
    if (mouse_down)
      {
        if (selected_marker == MARKER_NONE)
          return;

        const double sample_len_ms = m_sample->wav_data.samples().size() / m_sample->wav_data.mix_freq() * 1000.0;
        const double x_ms = sm_bound<double> (0, x / width * sample_len_ms, sample_len_ms);

        m_sample->set_marker (selected_marker, x_ms);

        /* enforce ordering constraints */
        vector<MarkerType> left, right;
        get_order (selected_marker, left, right);

        for (auto l : left)
          if (m_sample->get_marker (l) > x_ms)
            m_sample->set_marker (l, x_ms);

        for (auto r : right)
          if (m_sample->get_marker (r) < x_ms)
            m_sample->set_marker (r, x_ms);

        update();
      }
    else
      {
        MarkerType old_marker = selected_marker;
        selected_marker = find_marker_xy (x, y);

        if (selected_marker != old_marker)
          update();
      }
  }
  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
    selected_marker = find_marker_xy (x, y);

    update();
  }
  void
  leave_event() override
  {
    selected_marker = MARKER_NONE;
    update();
  }
  void
  set_sample (Sample *sample)
  {
    m_sample = sample;
    update();
  }
  void
  set_vzoom (double factor)
  {
    vzoom = factor;
    update();
  }
};

class MainWindow : public Window
{
  Instrument instrument;

  SampleWidget *sample_widget;
  void
  load_sample (const string& filename)
  {
    if (filename != "")
      instrument.add_sample (filename);
  }
  void
  on_samples_changed()
  {
    sample_combobox->clear();
    if (instrument.size() == 0)
      {
        sample_combobox->set_text ("");
      }
    for (size_t i = 0; i < instrument.size(); i++)
      {
        Sample *sample = instrument.sample (i);
        string text = string_printf ("%zd %d %s", i, sample->midi_note, sample->filename.c_str());

        sample_combobox->add_item (text);

        if (int (i) == instrument.selected())
          sample_combobox->set_text (text);
      }
    sample_widget->set_sample (instrument.sample (instrument.selected()));
  }
  ComboBox *sample_combobox;
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
    /* attach to model */
    connect (instrument.signal_samples_changed, this, &MainWindow::on_samples_changed);

    FixedGrid grid;

    MenuBar *menu_bar = new MenuBar (this);

    fill_zoom_menu (menu_bar->add_menu ("Zoom"));
    Menu *file_menu = menu_bar->add_menu ("File");

    MenuItem *add_item = file_menu->add_item ("Add Sample...");
    connect (add_item->signal_clicked, this, &MainWindow::on_add_sample_clicked);

    MenuItem *load_item = file_menu->add_item ("Load Instrument...");
    connect (load_item->signal_clicked, this, &MainWindow::on_load_clicked);

    MenuItem *save_item = file_menu->add_item ("Save Instrument...");
    connect (save_item->signal_clicked, this, &MainWindow::on_save_clicked);

    grid.add_widget (menu_bar, 1, 1, 91, 3);

    sample_combobox = new ComboBox (this);
    grid.add_widget (sample_combobox, 1, 5, 91, 3);

    connect (sample_combobox->signal_item_changed, this, &MainWindow::on_sample_changed);

    sample_scroll_view = new ScrollView (this);
    grid.add_widget (sample_scroll_view, 1, 8, 91, 46);

    sample_widget = new SampleWidget (sample_scroll_view);

    grid.add_widget (sample_widget, 1, 1, 100, 42);
    sample_scroll_view->set_scroll_widget (sample_widget, true, false, /* center_zoom */ true);

    instrument.load (test_sample);

    /*----- hzoom -----*/
    grid.add_widget (new Label (this, "HZoom"), 1, 54, 10, 3);
    Slider *hzoom_slider = new Slider (this, 0.0);
    grid.add_widget (hzoom_slider, 8, 54, 30, 3);
    connect (hzoom_slider->signal_value_changed, this, &MainWindow::on_update_hzoom);

    hzoom_label = new Label (this, "0");
    grid.add_widget (hzoom_label, 40, 54, 10, 3);

    /*----- vzoom -----*/
    grid.add_widget (new Label (this, "VZoom"), 1, 57, 10, 3);
    Slider *vzoom_slider = new Slider (this, 0.0);
    grid.add_widget (vzoom_slider, 8, 57, 30, 3);
    connect (vzoom_slider->signal_value_changed, this, &MainWindow::on_update_vzoom);

    vzoom_label = new Label (this, "0");
    grid.add_widget (vzoom_label, 40, 57, 10, 3);

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
  void
  on_save_clicked()
  {
    instrument.save ("/tmp/x.sminst");
  }
  void
  on_load_clicked()
  {
    instrument.load ("/tmp/x.sminst");
  }
  void
  on_sample_changed()
  {
    int idx = sample_combobox->current_index();
    if (idx >= 0)
      instrument.set_selected (idx);
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

  string fn = (argc > 1) ? argv[1] : "test.sminst";
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
