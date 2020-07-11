// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphwavsourceview.hh"
#include "sminsteditwindow.hh"
#include "smmorphplan.hh"
#include "smwavsetbuilder.hh"
#include "smuserinstrumentindex.hh"
#include "smzip.hh"

#include "smlabel.hh"
#include "smbutton.hh"
#include "smtimer.hh"
#include "smoperatorlayout.hh"
#include "smmessagebox.hh"

#include <unistd.h>
#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphWavSourceView::MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_wav_source, morph_plan_window),
  morph_wav_source (morph_wav_source),
  morph_wav_source_properties (morph_wav_source),
  pv_position (morph_wav_source_properties.position)
{
  instrument_label = new Label (body_widget, "Instrument");
  progress_bar = new ProgressBar (body_widget);
  instrument_combobox = new ComboBox (body_widget);
  Button *edit_button = new Button (body_widget, "Edit");

  update_instrument_list();

  op_layout.add_row (3, progress_bar, instrument_combobox, edit_button);

  // PLAY MODE
  ev_play_mode.add_item (MorphWavSource::PLAY_MODE_STANDARD,        "Standard");
  ev_play_mode.add_item (MorphWavSource::PLAY_MODE_CUSTOM_POSITION, "Custom Position");

  ComboBox *play_mode_combobox;
  play_mode_combobox = ev_play_mode.create_combobox (body_widget, morph_wav_source->play_mode(),
    [this, morph_wav_source] (int i) {
      morph_wav_source->set_play_mode (MorphWavSource::PlayMode (i));
      update_visible();
    });

  op_layout.add_row (3, new Label (body_widget, "Play Mode"), play_mode_combobox);

  // POSITION CONTROL INPUT
  position_control_combobox = cv_position_control.create_combobox (body_widget,
    morph_wav_source,
    morph_wav_source->position_control_type(),
    morph_wav_source->position_op());
  connect (cv_position_control.signal_control_changed, this, &MorphWavSourceView::on_position_control_changed);

  position_control_input_label = new Label (body_widget, "Position Ctrl");
  op_layout.add_row (3, position_control_input_label, position_control_combobox);

  // POSITION
  pv_position.init_ui (body_widget, op_layout);

  update_visible();

  instrument_label->set_x (0);
  instrument_label->set_y (0);
  instrument_label->set_width (progress_bar->width());
  instrument_label->set_height (progress_bar->height());

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
  return 5 + op_layout.height();
}

void
MorphWavSourceView::on_edit()
{
  SynthInterface *synth_interface = morph_plan_window->synth_interface();
  synth_interface->synth_inst_edit_update (true, nullptr, nullptr);

  Instrument *instrument = morph_wav_source->morph_plan()->project()->get_instrument (morph_wav_source);
  edit_instrument.reset (instrument->clone());

  InstEditWindow *inst_edit_window = new InstEditWindow (*window()->event_loop(), edit_instrument.get(), synth_interface, window());
  inst_edit_window->show();

  // after this line, inst edit window is owned by parent window
  window()->set_popup_window (inst_edit_window);

  inst_edit_window->set_close_callback ([this,  synth_interface]()
    {
      window()->set_popup_window (nullptr);
      synth_interface->synth_inst_edit_update (false, nullptr, nullptr);
      on_edit_close();
    });
}

string
MorphWavSourceView::modified_check (bool& wav_source_update, bool& user_inst_update)
{
  const char *change_text = "modified";
  if (!edit_instrument->size()) /* detect deletion: instrument without samples => deleted */
    {
      edit_instrument->clear(); // ensure same version as other deleted instruments
      change_text = "deleted";
    }

  auto project = morph_wav_source->morph_plan()->project();

  Instrument user_instrument;
  user_instrument.load (project->user_instrument_index()->filename (morph_wav_source->instrument()));

  string user_instrument_version = user_instrument.version();
  string wav_source_version = project->get_instrument (morph_wav_source)->version();
  string edit_instrument_version = edit_instrument->version();

  wav_source_update = edit_instrument_version != wav_source_version;
  user_inst_update = edit_instrument_version != user_instrument_version;

  return change_text;
}

void
MorphWavSourceView::on_edit_close()
{
  string full_name = string_printf ("%03d %s", morph_wav_source->instrument(), edit_instrument->name().c_str());

  bool wav_source_update, user_inst_update;
  string change_text = modified_check (wav_source_update, user_inst_update);
  if (wav_source_update || user_inst_update)
    {
      string message = string_printf (
          "Instrument \"%s\" has been %s.\n\n"
          "Press \"Save\" to update:\n",
          full_name.c_str(), change_text.c_str());

      if (wav_source_update)
        message += string_printf ("  - WavSource: %s\n", m_op->name().c_str());

      if (user_inst_update)
        message += string_printf ("  - User Instrument: %03d\n", morph_wav_source->instrument());

      auto confirm_box = new MessageBox (window(), "Save Instrument", message, MessageBox::SAVE | MessageBox::REVERT);

      confirm_box->run ([this](bool save_changes) { on_edit_save_changes (save_changes); });
    }
  else
    {
      on_edit_save_changes (false);
    }
}

void
MorphWavSourceView::on_edit_save_changes (bool save_changes)
{
  if (!save_changes) /* do not save changes */
    {
      edit_instrument.reset();
      return;
    }

  /* update copy in WavSource */
  auto        project  = morph_wav_source->morph_plan()->project();
  Instrument *instrument = project->get_instrument (morph_wav_source);

  ZipWriter edit_inst_writer;
  edit_instrument->save (edit_inst_writer);
  ZipReader edit_inst_reader (edit_inst_writer.data());
  instrument->load (edit_inst_reader);

  /* update on disk copy */
  string filename = project->user_instrument_index()->filename (morph_wav_source->instrument());
  if (instrument->size())
    {
      // create directory only when needed (on write)
      project->user_instrument_index()->create_instrument_dir();

      ZipWriter zip_writer (filename);
      instrument->save (zip_writer);
    }
  else
    {
      /* instrument without any samples -> remove */
      unlink (filename.c_str());
      instrument->clear();
    }
  edit_instrument.reset();
  update_instrument_list();
  project->rebuild (morph_wav_source);
  project->state_changed();
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
MorphWavSourceView::update_instrument_list()
{
  auto project  = morph_wav_source->morph_plan()->project();
  auto user_instrument_index = project->user_instrument_index();

  instrument_combobox->clear();
  for (int i = 1; i <= 128; i++)
    {
      string item = user_instrument_index->label (i);
      instrument_combobox->add_item (item);
    }
  Instrument *inst = project->get_instrument (morph_wav_source);
  if (inst && inst->size())
    instrument_combobox->set_text (string_printf ("%03d %s", morph_wav_source->instrument(), inst->name().c_str()));
  else
    instrument_combobox->set_text (string_printf ("%03d ---", morph_wav_source->instrument()));
}

void
MorphWavSourceView::update_visible()
{
  bool custom_position = morph_wav_source->play_mode() == MorphWavSource::PLAY_MODE_CUSTOM_POSITION;
  position_control_combobox->set_visible (custom_position);
  position_control_input_label->set_visible (custom_position);
  pv_position.set_visible (custom_position && morph_wav_source->position_control_type() == MorphWavSource::CONTROL_GUI);

  op_layout.activate();
  signal_size_changed();
}

void
MorphWavSourceView::on_position_control_changed()
{
  morph_wav_source->set_position_control_type_and_op (cv_position_control.control_type(), cv_position_control.op());

  update_visible();
}
