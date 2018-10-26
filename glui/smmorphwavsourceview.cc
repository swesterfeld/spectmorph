// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourceview.hh"
#include "sminsteditwindow.hh"
#include "smmorphplan.hh"
#include "smwavsetbuilder.hh"

#include "smlabel.hh"
#include "smbutton.hh"
#include "smoperatorlayout.hh"

#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphWavSourceView::MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_wav_source, morph_plan_window),
  morph_wav_source (morph_wav_source)
{
  OperatorLayout op_layout;

  Label *instrument_label = new Label (body_widget, "Instrument");
  Button *load_button = new Button (body_widget, "Load");
  Button *edit_button = new Button (body_widget, "Edit");

  op_layout.add_row (3, instrument_label, load_button, edit_button);
  op_layout.activate();

  connect (load_button->signal_clicked, this, &MorphWavSourceView::on_load);
  connect (edit_button->signal_clicked, this, &MorphWavSourceView::on_edit);
}

double
MorphWavSourceView::view_height()
{
  return 8;
}

void
MorphWavSourceView::on_load()
{
  window()->open_file_dialog ("Select SpectMorph Instrument to load", "SpectMorph Instrument files", "*.sminst", [=](string filename) {
    if (filename != "")
      {
        morph_wav_source->set_instrument (filename);
      /*
        Error error = load (filename);

        if (error != 0)
          {
            MessageBox::critical (this, "Error",
                                  string_locale_printf ("Import failed, unable to open file:\n'%s'\n%s.", filename.c_str(), sm_error_blurb (error)));
          }
          */
      }
  });
}

class NullBackend : public Backend
{
  MorphPlanWindow *morph_plan_window = nullptr;
  Timer *timer = nullptr;

  std::mutex result_mutex;
  bool have_result = false;
public:
  NullBackend (MorphPlanWindow *morph_plan_window) :
    morph_plan_window (morph_plan_window)
  {
  }
  void switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument = nullptr)
  {
    if (instrument)
      {
        WavSetBuilder *builder = new WavSetBuilder (instrument);

        add_builder (builder);
      }
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
    std::lock_guard<std::mutex> lg (result_mutex);

    WavSet wav_set;
    current_builder->get_result (wav_set);

    have_result = true;

    wav_set.save ("/tmp/midi_synth.smset");

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
    std::lock_guard<std::mutex> lg (result_mutex);

    return current_builder != nullptr;
  }
  int current_midi_note() {return 69;}
  void
  on_timer()
  {
    std::lock_guard<std::mutex> lg (result_mutex);
    if (have_result)
      {
        printf ("got result!\n");
        morph_plan_window->signal_inst_edit_update (true, "/tmp/midi_synth.smset", false);
        have_result = false;
      }
  }
};

void
MorphWavSourceView::on_edit()
{
  NullBackend *nb = new NullBackend (morph_plan_window);
  InstEditWindow *win = new InstEditWindow (morph_wav_source->instrument(), nb, window());

  morph_plan_window->signal_inst_edit_update (true, "", false);
  win->show();

  // after this line, rename window is owned by parent window
  window()->set_popup_window (win);

  win->set_close_callback ([&]()
    {
      morph_plan_window->signal_inst_edit_update (false, "", false);
      window()->set_popup_window (nullptr);
    });
}
