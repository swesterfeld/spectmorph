// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
#include "smbankeditwindow.hh"

#include <unistd.h>
#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphWavSourceView::MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_wav_source, morph_plan_window),
  morph_wav_source (morph_wav_source)
{
  user_instrument_index = morph_wav_source->morph_plan()->project()->user_instrument_index();

  bank_combobox = new ComboBox (body_widget);
  Button *banks_button = new Button (body_widget, "Banks...");

  instrument_label = new Label (body_widget, "Instrument");
  progress_bar = new ProgressBar (body_widget);
  instrument_combobox = new ComboBox (body_widget);
  Button *edit_button = new Button (body_widget, "Edit");

  on_banks_changed();
  update_instrument_list();
  update_instrument_labels();

  op_layout.add_row (3, new Label (body_widget, "Bank"), bank_combobox, banks_button);
  op_layout.add_row (3, progress_bar, instrument_combobox, edit_button);

  // PLAY MODE
  auto pv_play_mode = add_property_view (MorphWavSource::P_PLAY_MODE, op_layout);
  prop_play_mode = pv_play_mode->property();
  connect (prop_play_mode->signal_value_changed, this, &MorphWavSourceView::update_visible);

  // POSITION
  pv_position = add_property_view (MorphWavSource::P_POSITION, op_layout);

  // FORMANT CORRECT
  auto pv_formant_correct = add_property_view (MorphWavSource::P_FORMANT_CORRECTION, op_layout);
  prop_formant_correct = pv_formant_correct->property();
  connect (prop_formant_correct->signal_value_changed, this, &MorphWavSourceView::update_visible);
  pv_fuzzy_resynth = add_property_view (MorphWavSource::P_FUZZY_FREQS, op_layout);

  update_visible();

  instrument_label->set_x (0);
  instrument_label->set_y (3 * 8);
  instrument_label->set_width (progress_bar->width());
  instrument_label->set_height (progress_bar->height());

  Timer *timer = new Timer (this);
  timer->start (500);

  on_update_progress();

  connect (timer->signal_timeout, this, &MorphWavSourceView::on_update_progress);
  connect (instrument_combobox->signal_item_changed, this, &MorphWavSourceView::on_instrument_changed);
  connect (bank_combobox->signal_item_changed, this, &MorphWavSourceView::on_bank_changed);
  connect (edit_button->signal_clicked, this, &MorphWavSourceView::on_edit);
  connect (banks_button->signal_clicked, this, &MorphWavSourceView::on_edit_banks);

  connect (user_instrument_index->signal_banks_changed, this, &MorphWavSourceView::on_banks_changed);
  connect (user_instrument_index->signal_instrument_list_updated, this, &MorphWavSourceView::on_instrument_list_updated);
  connect (morph_wav_source->signal_labels_changed, this, &MorphWavSourceView::update_instrument_labels);
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
  const Instrument *instrument = morph_wav_source->morph_plan()->project()->lookup_instrument (morph_wav_source).instrument.get();

  InstEditWindow *inst_edit_window = new InstEditWindow (*window()->event_loop(), instrument, synth_interface, window());
  inst_edit_window->show();

  // after this line, inst edit window is owned by parent window
  window()->set_popup_window (inst_edit_window);

  inst_edit_window->set_close_callback ([this, inst_edit_window]()
    {
      edit_instrument = inst_edit_window->get_modified_instrument();
      window()->set_popup_window (nullptr);
      on_edit_close();
    });
}

void
MorphWavSourceView::on_edit_banks()
{
  auto bank_edit_window = new BankEditWindow (window(), "Edit Banks", morph_wav_source);

  // after this line, inst edit window is owned by parent window
  window()->set_popup_window (bank_edit_window);
  bank_edit_window->set_close_callback ([this]()
    {
      window()->set_popup_window (nullptr);
    });
  connect (bank_edit_window->signal_instrument_clicked, [this](const string& bank, int i)
    {
      morph_wav_source->set_bank_and_instrument (bank, i);
      window()->set_popup_window (nullptr);
    });
}

void
MorphWavSourceView::on_banks_changed()
{
  bank_combobox->clear();
  for (auto bank : user_instrument_index->list_banks())
    bank_combobox->add_item (bank);
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
  user_instrument.load (user_instrument_index->filename (morph_wav_source->bank(), morph_wav_source->instrument()));

  string user_instrument_version = user_instrument.version();
  string wav_source_version = project->lookup_instrument (morph_wav_source).instrument->version();
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
  if (save_changes)
    {
      Error error = user_instrument_index->update_instrument (morph_wav_source->bank(),
                                                              morph_wav_source->instrument(),
                                                             *edit_instrument);
      if (error)
        {
          auto filename = user_instrument_index->filename (morph_wav_source->bank(), morph_wav_source->instrument());
          MessageBox::critical (this, "Error",
                                string_locale_printf ("Saving User Instrument Failed:\n\n'%s'\n\n%s.", filename.c_str(), error.message()));
        }
    }
  edit_instrument.reset();
}

void
MorphWavSourceView::on_instrument_changed()
{
  morph_wav_source->set_bank_and_instrument (morph_wav_source->bank(), atoi (instrument_combobox->text().c_str()));
}

void
MorphWavSourceView::on_bank_changed()
{
  morph_wav_source->set_bank_and_instrument (bank_combobox->text(), 1);
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
MorphWavSourceView::on_instrument_list_updated (const string& bank)
{
  if (bank == morph_wav_source->bank())
    update_instrument_list();
}

void
MorphWavSourceView::update_instrument_list()
{
  instrument_combobox->clear();
  for (int i = 1; i <= 128; i++)
    {
      string item = user_instrument_index->label (morph_wav_source->bank(), i);
      instrument_combobox->add_item (item);
    }
}

void
MorphWavSourceView::update_instrument_labels()
{
  auto project  = morph_wav_source->morph_plan()->project();

  update_instrument_list();

  bank_combobox->set_text (morph_wav_source->bank());

  Instrument *inst = project->lookup_instrument (morph_wav_source).instrument.get();
  if (inst && inst->size())
    instrument_combobox->set_text (string_printf ("%03d %s", morph_wav_source->instrument(), inst->name().c_str()));
  else
    instrument_combobox->set_text (string_printf ("%03d ---", morph_wav_source->instrument()));
}

void
MorphWavSourceView::update_visible()
{
  bool custom_position = (prop_play_mode->get() == MorphWavSource::PLAY_MODE_CUSTOM_POSITION);
  pv_position->set_visible (custom_position);

  bool resynth = (prop_formant_correct->get() == FormantCorrection::MODE_HARMONIC_RESYNTHESIS);
  pv_fuzzy_resynth->set_visible (resynth);

  op_layout.activate();
  signal_size_changed();
}
