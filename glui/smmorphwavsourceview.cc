// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourceview.hh"
#include "smsamplewidget.hh"
#include "smmorphplan.hh"

#include "smlabel.hh"
#include "smbutton.hh"
#include "smoperatorlayout.hh"

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

void
MorphWavSourceView::on_edit()
{
  new SampleWidget (this);
}
