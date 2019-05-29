// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourceview.hh"
#include "sminsteditwindow.hh"
#include "smmorphplan.hh"
#include "smwavsetbuilder.hh"
#include "smuserinstrumentindex.hh"
#include "smzip.hh"

#include "smlabel.hh"
#include "smbutton.hh"
#include "smoperatorlayout.hh"

#include <unistd.h>
#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphWavSourceView::MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_wav_source, morph_plan_window),
  morph_wav_source (morph_wav_source)
{
  OperatorLayout op_layout;

  instrument_label = new Label (body_widget, "Instrument");
  progress_bar = new ProgressBar (body_widget);
  instrument_combobox = new ComboBox (body_widget);
  Button *edit_button = new Button (body_widget, "Edit");

  update_instrument_list();

  op_layout.add_row (3, progress_bar, instrument_combobox, edit_button);
  op_layout.activate();

  instrument_label->x = 0;
  instrument_label->y = 0;
  instrument_label->width = progress_bar->width;
  instrument_label->height = progress_bar->height;

  Timer *timer = new Timer (this);
  timer->start (500);

  on_update_progress();

  connect (timer->signal_timeout, this, &MorphWavSourceView::on_update_progress);
  connect (instrument_combobox->signal_item_changed, this, &MorphWavSourceView::on_instrument_changed);
  connect (edit_button->signal_clicked, this, &MorphWavSourceView::on_edit);
}

double
MorphWavSourceView::view_height()
{
  return 8;
}

void
MorphWavSourceView::on_edit()
{
  SynthInterface *synth_interface = morph_plan_window->synth_interface();
  synth_interface->synth_inst_edit_update (true, nullptr, false);

  Instrument *instrument = morph_wav_source->morph_plan()->project()->get_instrument (morph_wav_source);
  InstEditWindow *inst_edit_window = new InstEditWindow (*window()->event_loop(), instrument, synth_interface, window());

  inst_edit_window->show();

  // after this line, inst edit window is owned by parent window
  window()->set_popup_window (inst_edit_window);

  inst_edit_window->set_close_callback ([synth_interface,this,instrument]()
    {
      auto project = morph_wav_source->morph_plan()->project();
      window()->set_popup_window (nullptr);
      synth_interface->synth_inst_edit_update (false, nullptr, false);

      write_instrument();
      update_instrument_list();
      project->rebuild (morph_wav_source);
    });
}

void
MorphWavSourceView::on_instrument_changed()
{
  auto project = morph_wav_source->morph_plan()->project();

  Instrument *instrument = project->get_instrument (morph_wav_source);
  morph_wav_source->set_instrument (atoi (instrument_combobox->text().c_str()));

  Error error = instrument->load (project->user_instrument_index()->filename (morph_wav_source->instrument()));
  if (error)
    {
      /* most likely cause of error: this user instrument doesn't exist yet */
      instrument->clear();
    }
  project->rebuild (morph_wav_source);
}

void
MorphWavSourceView::on_update_progress()
{
  auto project = morph_wav_source->morph_plan()->project();
  bool rebuild_active = project->rebuild_active (morph_wav_source->object_id());

  if (rebuild_active)
    progress_bar->set_value (-1);
  else
    progress_bar->set_value (1);

  instrument_label->set_visible (!rebuild_active);
  progress_bar->set_visible (rebuild_active);
}


void
MorphWavSourceView::write_instrument()
{
  auto   project  = morph_wav_source->morph_plan()->project();
  string filename = project->user_instrument_index()->filename (morph_wav_source->instrument());

  Instrument *instrument = project->get_instrument (morph_wav_source);
  if (instrument->size())
    {
      ZipWriter zip_writer (filename);
      instrument->save (zip_writer);
    }
  else
    {
      /* instrument without any samples -> remove */
      unlink (filename.c_str());
      instrument->clear();
    }
}

void
MorphWavSourceView::update_instrument_list()
{
  auto user_instrument_index = morph_wav_source->morph_plan()->project()->user_instrument_index();

  instrument_combobox->clear();
  for (int i = 1; i <= 128; i++)
    {
      string item = user_instrument_index->label (i);
      instrument_combobox->add_item (item);

      if (i == morph_wav_source->instrument())
        instrument_combobox->set_text (item);
    }
}
