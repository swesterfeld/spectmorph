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
  /* create instrument in Project if WavSource doesn't have one */
  if (morph_wav_source->instrument() == 0)
    morph_wav_source->set_instrument (morph_wav_source->morph_plan()->project()->add_instrument());

  window()->open_file_dialog ("Select SpectMorph Instrument to load", "SpectMorph Instrument files", "*.sminst", [=](string filename) {
    if (filename != "")
      {
        Instrument *instrument = morph_wav_source->morph_plan()->project()->get_instrument (morph_wav_source->instrument());
        instrument->load (filename);

        morph_wav_source->morph_plan()->project()->rebuild (morph_wav_source->instrument());
      }
  });
}

void
MorphWavSourceView::on_edit()
{
  /* create instrument in Project if WavSource doesn't have one */
  if (morph_wav_source->instrument() == 0)
    morph_wav_source->set_instrument (morph_wav_source->morph_plan()->project()->add_instrument());

  SynthInterface *synth_interface = morph_plan_window->synth_interface();
  synth_interface->synth_inst_edit_update (true, nullptr, false);

  Instrument *instrument = morph_wav_source->morph_plan()->project()->get_instrument (morph_wav_source->instrument());
  InstEditWindow *inst_edit_window = new InstEditWindow (*window()->event_loop(), instrument, synth_interface, window());

  inst_edit_window->show();

  // after this line, inst edit window is owned by parent window
  window()->set_popup_window (inst_edit_window);

  inst_edit_window->set_close_callback ([synth_interface,this]()
    {
      window()->set_popup_window (nullptr);
      synth_interface->synth_inst_edit_update (false, nullptr, false);
      morph_wav_source->morph_plan()->project()->rebuild (morph_wav_source->instrument());
    });
}
