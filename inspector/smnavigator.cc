/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smnavigator.hh"
#include "smmicroconf.hh"
#include "smlivedecoder.hh"
#include "smindex.hh"

#include <assert.h>
#include <bse/bseloader.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QAbstractTableModel>
#include <QTreeView>
#include <QPushButton>

using namespace SpectMorph;

using std::vector;
using std::string;

namespace SpectMorph {

class TreeModel : public QAbstractTableModel
{
  WavSet *wset;
public:
  TreeModel (QWidget *parent, WavSet *wset);

  int
  rowCount (const QModelIndex& index) const
  {
    return index.isValid() ? 0 : wset->waves.size();
  }
  int
  columnCount (const QModelIndex& index) const
  {
    return index.isValid() ? 0 : 4;
  }
  QVariant
  data (const QModelIndex& index, int role) const
  {
    if (index.isValid() && role == Qt::DisplayRole)
      {
        const WavSetWave& wave = wset->waves[index.row()];

        switch (index.column())
          {
            case 0: return wset->waves[index.row()].midi_note;
            case 1: return wave.channel;
            case 2: return Birnet::string_printf ("%d..%d", wave.velocity_range_min, wave.velocity_range_max).c_str();
            case 3: return wave.path.c_str();
          }
      }
    return QVariant();
  }
  QVariant
  headerData (int section, Qt::Orientation orientation, int role) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      {
        switch (section)
          {
            case 0: return "Note";
            case 1: return "Ch";
            case 2: return "Range";
            case 3: return "Path";
          }
      }
    return QVariant();
  }
  void
  update_wset()
  {
    beginResetModel();
    endResetModel();
  }
};

TreeModel::TreeModel (QWidget *parent, WavSet *wset) :
  QAbstractTableModel (parent),
  wset (wset)
{
}

}

Navigator::Navigator (const string& filename)
{
  Index index;
  index.load_file (filename);

  smset_combobox = new QComboBox();

  smset_dir = index.smset_dir();
  for (vector<string>::const_iterator ii = index.smsets().begin(); ii != index.smsets().end(); ii++)
    smset_combobox->addItem (ii->c_str());
  connect (smset_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_combo_changed()));

  tree_model = new TreeModel (this, &wset);
  tree_view = new QTreeView();
  tree_view->setModel (tree_model);
  tree_view->setRootIsDecorated (false);
  connect (tree_view->selectionModel(), SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
           this, SLOT (on_selection_changed()));

  on_combo_changed();

  source_button = new QPushButton ("Source/Analysis");
  source_button->setCheckable (true);

  show_position_button = new QPushButton ("Show Position");
  show_position_button->setCheckable (true);
  connect (show_position_button, SIGNAL (clicked()), this, SLOT (on_show_position_changed()));

  show_analysis_button = new QPushButton ("Show Analysis");
  show_analysis_button->setCheckable (true);
  connect (show_analysis_button, SIGNAL (clicked()), this, SLOT (on_show_analysis_changed()));

  show_frequency_grid_button = new QPushButton ("Show Frequency Grid");
  show_frequency_grid_button->setCheckable (true);
  connect (show_frequency_grid_button, SIGNAL (clicked()), this, SLOT (on_show_frequency_grid_changed()));

  QPushButton *save_button = new QPushButton ("Save");

  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget (smset_combobox);
  vbox->addWidget (tree_view);
  vbox->addWidget (source_button);
  vbox->addWidget (show_position_button);
  vbox->addWidget (show_analysis_button);
  vbox->addWidget (show_frequency_grid_button);
  vbox->addWidget (save_button);

  setLayout (vbox);

  m_fft_param_window = new FFTParamWindow();
  m_display_param_window = new DisplayParamWindow();

  player_window = new PlayerWindow (this);
  sample_window = new SampleWindow (this);
  connect (sample_window, SIGNAL (next_sample()), this, SLOT (on_next_sample()));

  time_freq_window = new TimeFreqWindow (this);
  spectrum_window = new SpectrumWindow (this);
  lpc_window = new LPCWindow();

  spectrum_window->set_spectrum_model (time_freq_window->time_freq_view());
  lpc_window->set_lpc_model (time_freq_window->time_freq_view());

  connect (this, SIGNAL (dhandle_changed()), sample_window, SLOT (on_dhandle_changed()));
  connect (this, SIGNAL (dhandle_changed()), time_freq_window, SLOT (on_dhandle_changed()));
  connect (this, SIGNAL (show_position_changed()), time_freq_window, SLOT (on_position_changed()));
  connect (this, SIGNAL (show_analysis_changed()), time_freq_window, SLOT (on_analysis_changed()));
  connect (this, SIGNAL (show_frequency_grid_changed()), time_freq_window, SLOT (on_frequency_grid_changed()));
}

bool
Navigator::handle_close_event()
{
  player_window->close();
  sample_window->close();
  time_freq_window->close();
  spectrum_window->close();
  m_fft_param_window->close();
  m_display_param_window->close();
  lpc_window->close();

  return true;
}

void
Navigator::on_combo_changed()
{
  string new_filename = smset_dir + "/" + smset_combobox->currentText().toLatin1().data();
#if 0
  if (wset_edit && new_filename != wset_filename)
    {
      Gtk::MessageDialog dlg (Birnet::string_printf ("You changed instrument '%s' - if you switch instruments now your changes will be lost.", wset_filename.c_str()),
                              false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_CANCEL);
      dlg.add_button ("Continue without saving", Gtk::RESPONSE_ACCEPT);
      if (dlg.run() != Gtk::RESPONSE_ACCEPT)
        {
          smset_combobox.set_active_text (wset_active_text);
          return;
        }
    }
#endif
  wset_filename = new_filename;
  wset_edit = false;
  wset_active_text = smset_combobox->currentText().toLatin1().data();
  BseErrorType error = wset.load (wset_filename);
  if (error)
    {
      fprintf (stderr, "sminspector: can't open input file: %s: %s\n", wset_filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  audio = NULL;
  dhandle = NULL;

  tree_model->update_wset();
  for (int column = 0; column < 4; column++)
    tree_view->resizeColumnToContents (column);

  emit dhandle_changed();
}

void
Navigator::on_selection_changed()
{
  int row = tree_view->selectionModel()->currentIndex().row();
  int column = tree_view->selectionModel()->currentIndex().column();

  size_t i = row;
  assert (i < wset.waves.size());

  int channel = wset.waves[i].channel;

  audio = wset.waves[i].audio;
  assert (wset.waves[i].audio);

  if (spectmorph_signal_active())
    {
      LiveDecoder decoder (&wset);
      decoder.retrigger (channel, audio->fundamental_freq, 127, audio->mix_freq);
      decoded_samples.resize (audio->sample_count);
      decoder.process (decoded_samples.size(), 0, 0, &decoded_samples[0]);
      dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, decoded_samples.size(), &decoded_samples[0], NULL);
    }
  else
    {
      dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, audio->original_samples.size(), &audio->original_samples[0], NULL);
    }
  emit dhandle_changed();
}

void
Navigator::on_view_sample()
{
  sample_window->show();
}

void
Navigator::on_view_player()
{
  player_window->show();
}

void
Navigator::on_view_time_freq()
{
  time_freq_window->show();
}

void
Navigator::on_view_fft_params()
{
  m_fft_param_window->show();
}

Audio *
Navigator::get_audio()
{
  return audio;
}

GslDataHandle *
Navigator::get_dhandle()
{
  return dhandle;
}

bool
Navigator::spectmorph_signal_active()
{
  return source_button->isChecked();
}

FFTParamWindow*
Navigator::fft_param_window()
{
  return m_fft_param_window;
}


#if 0
Navigator::Navigator (const string& filename) :
  dhandle (NULL),
  audio (NULL),
  spectrum_window (this),
  sample_window (this),
  time_freq_window (this),
  player_window (this)
{
  Index index;
  index.load_file (filename);

  smset_dir = index.smset_dir();
  for (vector<string>::const_iterator ii = index.smsets().begin(); ii != index.smsets().end(); ii++)
    smset_combobox.append_text (*ii);

  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("ViewMenu", "View"));
  ref_action_group->add (Gtk::Action::create ("ViewTimeFreq", "Time/Frequency View"),
                         sigc::mem_fun (*this, &Navigator::on_view_time_freq));
  ref_action_group->add (Gtk::Action::create ("ViewSample", "Sample View"),
                         sigc::mem_fun (*this, &Navigator::on_view_sample));
  ref_action_group->add (Gtk::Action::create ("ViewSpectrum", "Spectrum View"),
                         sigc::mem_fun (*this, &Navigator::on_view_spectrum));
  ref_action_group->add (Gtk::Action::create ("ViewLPC", "LPC View"),
                         sigc::mem_fun (*this, &Navigator::on_view_lpc));
  ref_action_group->add (Gtk::Action::create ("ViewFFTParams", "FFT Params"),
                         sigc::mem_fun (*this, &Navigator::on_view_fft_params));
  ref_action_group->add (Gtk::Action::create ("ViewDisplayParams", "Display Params"),
                         sigc::mem_fun (*this, &Navigator::on_view_display_params));
  ref_action_group->add (Gtk::Action::create ("ViewPlayer", "Player"),
                         sigc::mem_fun (*this, &Navigator::on_view_player));

  ref_ui_manager = Gtk::UIManager::create();
  ref_ui_manager-> insert_action_group (ref_action_group);
  add_accel_group (ref_ui_manager->get_accel_group());

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='ViewMenu'>"
    "      <menuitem action='ViewTimeFreq' />"
    "      <menuitem action='ViewSample' />"
    "      <menuitem action='ViewSpectrum' />"
    "      <menuitem action='ViewLPC' />"
    "      <menuitem action='ViewFFTParams' />"
    "      <menuitem action='ViewDisplayParams' />"
    "      <menuitem action='ViewPlayer' />"
    "    </menu>"
    "  </menubar>"
    "</ui>";
  try
    {
      ref_ui_manager->add_ui_from_string (ui_info);
    }
  catch (const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " << ex.what();
    }

  Gtk::Widget *menu_bar = ref_ui_manager->get_widget ("/MenuBar");
  if (menu_bar)
    index_vbox.pack_start (*menu_bar, Gtk::PACK_SHRINK);

  set_title ("Instrument Navigator");
  set_border_width (10);
  set_default_size (300, 600);
  index_vbox.set_spacing (10);
  index_vbox.pack_start (smset_combobox, Gtk::PACK_SHRINK);
  ref_tree_model = Gtk::ListStore::create (audio_chooser_cols);
  tree_view.set_model (ref_tree_model);
  tree_view_scrolled_window.add (tree_view);
  index_vbox.pack_start (tree_view_scrolled_window);

  tree_view.append_column ("Note", audio_chooser_cols.col_note);
  tree_view.append_column ("Ch", audio_chooser_cols.col_channel);
  tree_view.append_column ("Range", audio_chooser_cols.col_range);
  tree_view.append_column ("Path", audio_chooser_cols.col_path);

  ref_tree_selection = tree_view.get_selection();
  ref_tree_selection->signal_changed().connect (sigc::mem_fun (*this, &Navigator::on_selection_changed));

  source_button.set_label ("Source/Analysis");
  index_vbox.pack_start (source_button, Gtk::PACK_SHRINK);
  source_button.signal_toggled().connect (sigc::mem_fun (*this, &Navigator::on_selection_changed));
  add (index_vbox);

  show_position_button.set_label ("Show Position");
  index_vbox.pack_start (show_position_button, Gtk::PACK_SHRINK);
  show_position_button.signal_toggled().connect (sigc::mem_fun (*this, &Navigator::on_show_position_changed));

  show_analysis_button.set_label ("Show Analysis");
  index_vbox.pack_start (show_analysis_button, Gtk::PACK_SHRINK);
  show_analysis_button.signal_toggled().connect (sigc::mem_fun (*this, &Navigator::on_show_analysis_changed));

  show_frequency_grid_button.set_label ("Show Frequency Grid");
  index_vbox.pack_start (show_frequency_grid_button, Gtk::PACK_SHRINK);
  show_frequency_grid_button.signal_toggled().connect (sigc::mem_fun (*this, &Navigator::on_show_frequency_grid_changed));

  save_button.set_label ("Save");
  index_vbox.pack_start (save_button, Gtk::PACK_SHRINK);
  save_button.signal_clicked().connect (sigc::mem_fun (*this, &Navigator::on_save_clicked));

  show();
  show_all_children();
  smset_combobox.signal_changed().connect (sigc::mem_fun (*this, &Navigator::on_combo_changed));

  wset_edit = false;

  signal_dhandle_changed.connect (sigc::mem_fun (time_freq_window, &TimeFreqWindow::on_dhandle_changed));
  signal_dhandle_changed.connect (sigc::mem_fun (sample_window, &SampleWindow::on_dhandle_changed));
  signal_show_position_changed.connect (sigc::mem_fun (time_freq_window, &TimeFreqWindow::on_position_changed));
  signal_show_analysis_changed.connect (sigc::mem_fun (time_freq_window, &TimeFreqWindow::on_analysis_changed));
  signal_show_frequency_grid_changed.connect (sigc::mem_fun (time_freq_window, &TimeFreqWindow::on_frequency_grid_changed));

  sample_window.sample_view().signal_audio_edit.connect (sigc::mem_fun (*this, &Navigator::on_audio_edit));
  sample_window.signal_next_sample.connect (sigc::mem_fun (*this, &Navigator::on_next_sample));

  spectrum_window.set_spectrum_model (*time_freq_window.time_freq_view());
  lpc_window.set_lpc_model (*time_freq_window.time_freq_view());
}

void
Navigator::on_selection_changed()
{
  Gtk::TreeModel::iterator iter = ref_tree_selection->get_selected();
  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;
      size_t i = row[audio_chooser_cols.col_wave_nr];
      assert (i < wset.waves.size());

      int channel = row[audio_chooser_cols.col_channel];

      audio = wset.waves[i].audio;
      assert (wset.waves[i].audio);

      if (spectmorph_signal_active())
        {
          LiveDecoder decoder (&wset);
          decoder.retrigger (channel, audio->fundamental_freq, 127, audio->mix_freq);
          decoded_samples.resize (audio->sample_count);
          decoder.process (decoded_samples.size(), 0, 0, &decoded_samples[0]);
          dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, decoded_samples.size(), &decoded_samples[0], NULL);
        }
      else
        {
          dhandle = gsl_data_handle_new_mem (1, 32, audio->mix_freq, 440, audio->original_samples.size(), &audio->original_samples[0], NULL);
        }
      signal_dhandle_changed();
    }
}

bool
Navigator::spectmorph_signal_active()
{
  return source_button.get_active();
}

void
Navigator::on_combo_changed()
{
  string new_filename = smset_dir + "/" + smset_combobox.get_active_text().c_str();
  if (wset_edit && new_filename != wset_filename)
    {
      Gtk::MessageDialog dlg (Birnet::string_printf ("You changed instrument '%s' - if you switch instruments now your changes will be lost.", wset_filename.c_str()),
                              false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_CANCEL);
      dlg.add_button ("Continue without saving", Gtk::RESPONSE_ACCEPT);
      if (dlg.run() != Gtk::RESPONSE_ACCEPT)
        {
          smset_combobox.set_active_text (wset_active_text);
          return;
        }
    }
  wset_filename = new_filename;
  wset_edit = false;
  wset_active_text = smset_combobox.get_active_text();
  BseErrorType error = wset.load (wset_filename);
  if (error)
    {
      fprintf (stderr, "sminspector: can't open input file: %s: %s\n", wset_filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  audio = NULL;
  dhandle = NULL;

  signal_dhandle_changed();

  ref_tree_model->clear();
  for (size_t i = 0; i < wset.waves.size(); i++)
    {
      const WavSetWave& wave = wset.waves[i];

      Gtk::TreeModel::Row row = *(ref_tree_model->append());
      row[audio_chooser_cols.col_note] = wave.midi_note;
      row[audio_chooser_cols.col_channel] = wave.channel;
      row[audio_chooser_cols.col_range] = Birnet::string_printf ("%d..%d", wave.velocity_range_min, wave.velocity_range_max);
      row[audio_chooser_cols.col_path] = wave.path;
      row[audio_chooser_cols.col_wave_nr] = i;
    }
}
#endif

void
Navigator::on_show_position_changed()
{
  emit show_position_changed();
}

void
Navigator::on_show_analysis_changed()
{
  emit show_analysis_changed();
}

void
Navigator::on_show_frequency_grid_changed()
{
  emit show_frequency_grid_changed();
}

bool
Navigator::get_show_position()
{
  return show_position_button->isChecked();
}

bool
Navigator::get_show_analysis()
{
  return show_analysis_button->isChecked();
}

bool
Navigator::get_show_frequency_grid()
{
  return show_frequency_grid_button->isChecked();
}

#if 0
void
Navigator::on_save_clicked()
{
  if (wset_filename != "")
    {
      BseErrorType error = wset.save (wset_filename);
      if (error)
        {
          fprintf (stderr, "sminspector: can't write output file: %s: %s\n", wset_filename.c_str(), bse_error_blurb (error));
          exit (1);
        }
      wset_edit = false;
    }

}

void
Navigator::on_audio_edit()
{
  wset_edit = true;
}
#endif

void
Navigator::on_next_sample()
{
  QModelIndex index = tree_view->selectionModel()->currentIndex();

  if (index.isValid())
    index = tree_model->index (index.row() + 1, 0);  // select next sample
  else
    index = tree_model->index (0, 0); // select first sample if nothing was selected

  if (index.isValid())
    tree_view->selectionModel()->setCurrentIndex (index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

#if 0
void
Navigator::on_view_time_freq()
{
  time_freq_window.show();
}
#endif

void
Navigator::on_view_display_params()
{
  m_display_param_window->show();
}

void
Navigator::on_view_spectrum()
{
  spectrum_window->show();
}

void
Navigator::on_view_lpc()
{
  lpc_window->show();
}

DisplayParamWindow *
Navigator::display_param_window()
{
  return m_display_param_window;
}

#if 0
bool
Navigator::on_delete_event (GdkEventAny* event)
{
  if (wset_edit)
    {
      Gtk::MessageDialog dlg (Birnet::string_printf ("You changed instrument '%s' - if you quit now your changes will be lost.", wset_filename.c_str()),
                              false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_CANCEL);
      dlg.add_button ("Quit without saving", Gtk::RESPONSE_ACCEPT);
      if (dlg.run() != Gtk::RESPONSE_ACCEPT)
        return true;    // postpone quit
    }
  return false;         // -> quit
}
#endif
