// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "spectmorphglui.hh"
#include "smpugixml.hh"
#include "sminstrument.hh"
#include "smwavsetbuilder.hh"
#include <thread>

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

enum class PlayMode
{
  SAMPLE,
  SPECTMORPH,
  REFERENCE
};

#if 0
class WavSetCreator
{
  WavData wav_data;
  WavSet  wav_set;
  int     midi_note;
  double  loop_start_ms;
  double  loop_end_ms;
  Sample::Loop loop;

  void
  apply_loop_settings()
  {
    wav_set.load ("/tmp/x.smset");

    assert (wav_set.waves.size() == 1);

    // FIXME! account for zero_padding at start of sample
    const int loop_start = loop_start_ms / wav_set.waves[0].audio->frame_step_ms;
    const int loop_end   = loop_end_ms / wav_set.waves[0].audio->frame_step_ms;
    Audio *audio = wav_set.waves[0].audio;

    if (loop == Sample::Loop::NONE)
      {
        audio->loop_type = Audio::LOOP_NONE;
        audio->loop_start = 0;
        audio->loop_end = 0;
      }
    else if (loop == Sample::Loop::FORWARD)
      {
        audio->loop_type = Audio::LOOP_FRAME_FORWARD;
        audio->loop_start = loop_start;
        audio->loop_end = loop_end;
      }
    else if (loop == Sample::Loop::PING_PONG)
      {
        audio->loop_type = Audio::LOOP_FRAME_PING_PONG;
        audio->loop_start = loop_start;
        audio->loop_end = loop_end;
      }
    else if (loop == Sample::Loop::SINGLE_FRAME)
      {
        audio->loop_type = Audio::LOOP_FRAME_FORWARD;

        // single frame loop
        audio->loop_start = loop_start;
        audio->loop_end   = loop_start;
      }

    string lt_string;
    bool have_loop_type = Audio::loop_type_to_string (audio->loop_type, lt_string);
    if (have_loop_type)
      printf ("loop-type  = %s\n", lt_string.c_str());

    printf ("loop-start = %d\n", audio->loop_start);
    printf ("loop-end   = %d\n", audio->loop_end);

    wav_set.save ("/tmp/x.smset");
  }
public:
  WavSetCreator (const Sample *sample, WavData& wav_data) :
    wav_data (wav_data)
  {
    midi_note = sample->midi_note();

    const double clip_adjust = std::max (0.0, sample->get_marker (MARKER_CLIP_START));

    loop = sample->loop();
    loop_start_ms = sample->get_marker (MARKER_LOOP_START) - clip_adjust;
    loop_end_ms = sample->get_marker (MARKER_LOOP_END) - clip_adjust;
  }
  void
  run()
  {
    wav_data.save ("/tmp/x.wav");
    string cmd = string_printf ("smenccache /tmp/x.wav /tmp/x.sm -m %d -O1 -s", midi_note);
    printf ("# %s\n", cmd.c_str());
    system (cmd.c_str());

    WavSetWave new_wave;
    new_wave.midi_note = midi_note;
    new_wave.path = "/tmp/x.sm";
    new_wave.channel = 0;
    new_wave.velocity_range_min = 0;
    new_wave.velocity_range_max = 127;

    wav_set.waves.push_back (new_wave);
    wav_set.save ("/tmp/x.smset", true); // link wavset

    apply_loop_settings();
  }
  string
  filename()
  {
    return "/tmp/x.smset";
  }
};
#endif

class JackBackend
{
  double jack_mix_freq;
  jack_port_t *input_port;
  jack_port_t *output_port;

  std::mutex decoder_mutex;
  std::unique_ptr<LiveDecoder> decoder;
  double decoder_factor = 0;
  int m_current_midi_note = -1;
  WavSet wav_set;
public:

  static int
  jack_process (jack_nframes_t nframes, void *arg)
  {
    JackBackend *instance = reinterpret_cast<JackBackend *> (arg);
    return instance->process (nframes);
  }

  double
  note_to_freq (int note)
  {
    return 440 * exp (log (2) * (note - 69) / 12.0);
  }
  int
  process (jack_nframes_t nframes)
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);
    float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);

    void* port_buf = jack_port_get_buffer (input_port, nframes);
    jack_nframes_t event_count = jack_midi_get_event_count (port_buf);
    if (event_count)
      for (jack_nframes_t event_index = 0; event_index < event_count; event_index++)
      {
        jack_midi_event_t    in_event;
        jack_midi_event_get (&in_event, port_buf, event_index);

        if (in_event.buffer[0] == 0x90)
          {
            const int note = in_event.buffer[1];
            const double freq = note_to_freq (note);

            if (decoder)
              {
                decoder->retrigger (0, freq, 127, 48000);
                m_current_midi_note = note;
              }
            decoder_factor = 1;
          }
        if (in_event.buffer[0] == 0x80)
          {
            decoder_factor = 0;
            m_current_midi_note = -1;
          }
        //midi_synth->add_midi_event (in_event.time, in_event.buffer);
      }

    if (decoder)
      {
        decoder->process (nframes, nullptr, &audio_out[0]);
        for (uint i = 0; i < nframes; i++)
          audio_out[i] *= decoder_factor;
      }
    else
      {
        for (uint i = 0; i < nframes; i++)
          audio_out[i] = 0;
      }
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

  void
  switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument = nullptr)
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

    if (play_mode == PlayMode::SAMPLE)
      {
        wav_set.clear();

        WavSetWave new_wave;
        new_wave.midi_note = sample->midi_note();
        // new_wave.path = "..";
        new_wave.channel = 0;
        new_wave.velocity_range_min = 0;
        new_wave.velocity_range_max = 127;

        Audio audio;
        audio.mix_freq = sample->wav_data.mix_freq();
        audio.fundamental_freq = note_to_freq (sample->midi_note());
        audio.original_samples = sample->wav_data.samples();
        new_wave.audio = audio.clone();

        wav_set.waves.push_back (new_wave);

        decoder.reset (new LiveDecoder (&wav_set));
        decoder->enable_original_samples (true);
      }
    else if (play_mode == PlayMode::REFERENCE)
      {
        Index index;
        index.load_file ("instruments:standard");

        string smset_dir = index.smset_dir();

        decoder.reset (new LiveDecoder (WavSetRepo::the()->get (smset_dir + "/synth-saw.smset")));
      }
    else if (play_mode == PlayMode::SPECTMORPH)
      {
#if 0
        assert (sample->wav_data.n_channels() == 1);

        vector<float> samples = sample->wav_data.samples();
        vector<float> clipped_samples;
        for (size_t i = 0; i < samples.size(); i++)
          {
            double pos_ms = i * (1000.0 / sample->wav_data.mix_freq());
            if (pos_ms >= sample->get_marker (MARKER_CLIP_START))
              {
                /* if we have a loop, the loop end determines the real end of the recording */
                if (sample->loop() != Sample::Loop::NONE || pos_ms <= sample->get_marker (MARKER_CLIP_END))
                  clipped_samples.push_back (samples[i]);
              }
          }

        WavData wd_clipped;
        wd_clipped.load (clipped_samples, 1, sample->wav_data.mix_freq());

        WavSetBuilder *wbuilder = new WavSetBuilder (sample, wd_clipped);

        add_builder (wbuilder);
#endif
        WavSetBuilder *wbuilder = new WavSetBuilder (instrument);

        add_builder (wbuilder);
      }
    else
      {
        decoder.reset (nullptr); // not yet implemented
      }
  }
  int
  current_midi_note()
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

    return m_current_midi_note;
  }
  WavSetBuilder *current_builder = nullptr;
  WavSetBuilder *next_builder = nullptr;
  void
  add_builder (WavSetBuilder *builder)
  {
    if (current_builder)
      {
        if (next_builder) /* kill and overwrite obsolete next builder */
          delete next_builder;

        next_builder = builder;
      }
    else
      {
        start_as_current (builder);
      }
  }
  void
  start_as_current (WavSetBuilder *builder)
  {
    current_builder = builder;
    new std::thread ([this] () {
      current_builder->run();

      finish_current_builder();
    });
  }
  void
  finish_current_builder()
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

    current_builder->get_result (wav_set);

    decoder.reset (new LiveDecoder (&wav_set));

    delete current_builder;
    current_builder = nullptr;

    if (next_builder)
      {
        WavSetBuilder *builder = next_builder;

        next_builder = nullptr;
        start_as_current (builder);
      }
  }
  bool
  have_builder()
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

    return current_builder != nullptr;
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
    if (m_sample->loop() == Sample::Loop::FORWARD || m_sample->loop() == Sample::Loop::PING_PONG)
      {
        cairo_rectangle (cr, loop_start_x, 0, loop_end_x - loop_start_x, height);
        cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.25);
        cairo_fill (cr);
      }

    /* darken widget before and after clip region */
    cairo_rectangle (cr, 0, 0, clip_start_x, height);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.25);
    cairo_fill (cr);

    double effective_end_x = clip_end_x;
    if (m_sample->loop() == Sample::Loop::FORWARD || m_sample->loop() == Sample::Loop::PING_PONG)
      effective_end_x = loop_end_x;
    if (m_sample->loop() == Sample::Loop::SINGLE_FRAME)
      effective_end_x = loop_start_x;

    cairo_rectangle (cr, effective_end_x, 0, width - effective_end_x, height);
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
            double c = 0;

            if (m_sample->loop() == Sample::Loop::NONE)
              continue;
            if (m_sample->loop() == Sample::Loop::SINGLE_FRAME) // center rect for single frame loop
              c = 5;

            rect = Rect (marker_x - c, 0, 10, 10);
            color = Color (0.7, 0.7, 1);

          }
        else if (m == MARKER_LOOP_END)
          {
            if (m_sample->loop() == Sample::Loop::NONE || m_sample->loop() == Sample::Loop::SINGLE_FRAME)
              continue;

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
            if (m_sample->loop() != Sample::Loop::NONE)
              continue;

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
  void
  update_markers()
  {
    update();
  }
};

class MainWindow : public Window
{
  Instrument instrument;

  SampleWidget *sample_widget;
  ComboBox *midi_note_combobox = nullptr;

  string
  note_to_text (int i)
  {
    vector<string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return string_printf ("%d  :  %s%d", i, note_name[i % 12].c_str(), i / 12 - 2);
  }
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
        string text = string_printf ("%s  :  %s", note_to_text (sample->midi_note()).c_str(), sample->filename.c_str());

        sample_combobox->add_item (text);

        if (int (i) == instrument.selected())
          sample_combobox->set_text (text);
      }
    Sample *sample = instrument.sample (instrument.selected());
    sample_widget->set_sample (sample);
    midi_note_combobox->set_enabled (sample != nullptr);
    sample_combobox->set_enabled (sample != nullptr);
    play_mode_combobox->set_enabled (sample != nullptr);
    loop_combobox->set_enabled (sample != nullptr);
    if (!sample)
      {
        midi_note_combobox->set_text ("");
        loop_combobox->set_text ("");
      }
    else
      {
        midi_note_combobox->set_text (note_to_text (sample->midi_note()));
        loop_combobox->set_text (loop_to_text (sample->loop()));
      }
    if (sample)
      {
        jack_backend->switch_to_sample (sample, play_mode, &instrument);
      }
  }
  void
  on_marker_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    if (sample)
      {
        sample_widget->update_markers();
        jack_backend->switch_to_sample (sample, play_mode, &instrument);
      }
  }
  ComboBox *sample_combobox;
  ScrollView *sample_scroll_view;
  Label *hzoom_label;
  Label *vzoom_label;
  JackBackend *jack_backend;
  PlayMode play_mode = PlayMode::SAMPLE;
  ComboBox *play_mode_combobox;
  ComboBox *loop_combobox;
  Led *led;
  Label *playing_label;

  Sample::Loop
  text_to_loop (const std::string& text)
  {
    for (int i = 0; ; i++)
      {
        string txt = loop_to_text (Sample::Loop (i));

        if (txt == text)
          return Sample::Loop (i);
        if (txt == "")
          return Sample::Loop (0);
      }
  }
  string
  loop_to_text (const Sample::Loop loop)
  {
    switch (loop)
      {
        case Sample::Loop::NONE:        return "None";
        case Sample::Loop::FORWARD:     return "Forward";
        case Sample::Loop::PING_PONG:   return "Ping Pong";
        case Sample::Loop::SINGLE_FRAME:return "Single Frame";
      }
    return ""; /* not found */
  }
public:
  void
  on_add_sample_clicked()
  {
    open_file_dialog ("Select Sample to load", "Wav Files", "*.wav", [=](string filename) {
      load_sample (filename);
    });
  }
  MainWindow (const string& test_sample, JackBackend *jack_backend) :
    Window ("SpectMorph - Instrument Editor", win_width, win_height),
    jack_backend (jack_backend)
  {
    /* attach to model */
    connect (instrument.signal_samples_changed, this, &MainWindow::on_samples_changed);
    connect (instrument.signal_marker_changed, this, &MainWindow::on_marker_changed);

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

    /*---- midi_note ---- */
    midi_note_combobox = new ComboBox (this);
    connect (midi_note_combobox->signal_item_changed, this, &MainWindow::on_midi_note_changed);

    for (int i = 127; i >= 0; i--)
      midi_note_combobox->add_item (note_to_text (i));

    grid.add_widget (new Label (this, "Midi Note"), 1, 60, 10, 3);
    grid.add_widget (midi_note_combobox, 8, 60, 20, 3);

    /*---- loop mode ----*/

    loop_combobox = new ComboBox (this);
    connect (loop_combobox->signal_item_changed, this, &MainWindow::on_loop_changed);

    loop_combobox->add_item (loop_to_text (Sample::Loop::NONE));
    loop_combobox->set_text (loop_to_text (Sample::Loop::NONE));
    loop_combobox->add_item (loop_to_text (Sample::Loop::FORWARD));
    loop_combobox->add_item (loop_to_text (Sample::Loop::PING_PONG));
    loop_combobox->add_item (loop_to_text (Sample::Loop::SINGLE_FRAME));

    grid.add_widget (new Label (this, "Loop"), 1, 63, 10, 3);
    grid.add_widget (loop_combobox, 8, 63, 20, 3);

    /*--- play mode ---*/
    play_mode_combobox = new ComboBox (this);
    connect (play_mode_combobox->signal_item_changed, this, &MainWindow::on_play_mode_changed);
    grid.add_widget (new Label (this, "Play Mode"), 60, 54, 10, 3);
    grid.add_widget (play_mode_combobox, 68, 54, 20, 3);
    play_mode_combobox->add_item ("Original Sample");
    play_mode_combobox->set_text ("Original Sample"); // default
    play_mode_combobox->add_item ("SpectMorph Instrument");
    play_mode_combobox->add_item ("Reference Instrument");

    /*--- led ---*/
    led = new Led (this, false);
    grid.add_widget (new Label (this, "Analyzing"), 70, 64, 10, 3);
    grid.add_widget (led, 77, 64.5, 2, 2);

    /*--- playing ---*/
    playing_label = new Label (this, "");
    grid.add_widget (new Label (this, "Playing"), 70, 67, 10, 3);
    grid.add_widget (playing_label, 77, 67, 10, 3);

    instrument.load (test_sample);

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
  void
  on_midi_note_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    if (!sample)
      return;
    for (int i = 0; i < 128; i++)
      {
        if (midi_note_combobox->text() == note_to_text (i))
          {
            sample->set_midi_note (i);
          }
      }
  }
  void
  on_play_mode_changed()
  {
    int idx = play_mode_combobox->current_index();
    if (idx >= 0)
      {
        play_mode = static_cast <PlayMode> (idx);

        // this may do a little more than we need, but it updates play_mode
        // in the backend
        on_samples_changed();
      }
  }
  void
  on_loop_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    sample->set_loop (text_to_loop (loop_combobox->text()));
  }
  void
  update_led()
  {
    led->set_on (jack_backend->have_builder());

    int note = jack_backend->current_midi_note();
    playing_label->set_text (note >= 0 ? note_to_text (note) : "---");
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
  MainWindow window (fn, &jack_backend);

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_event_fps();
      window.process_events();
      window.update_led();
    }
  jack_deactivate (client);
}
