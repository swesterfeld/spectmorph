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

#ifndef SPECTMORPH_NAVIGATOR_HH
#define SPECTMORPH_NAVIGATOR_HH

#include "smwavset.hh"
#include "smtimefreqwindow.hh"
#include "smplayerwindow.hh"
#include "smlpcwindow.hh"
#include "smdisplayparamwindow.hh"

#include <QComboBox>

namespace SpectMorph {

class Navigator : public QWidget
{
private:
  std::string           smset_dir;
  QComboBox            *smset_combobox;

public:
  Navigator (const std::string& filename);
#if 0
  Glib::RefPtr<Gtk::UIManager>    ref_ui_manager;
  Glib::RefPtr<Gtk::ActionGroup>  ref_action_group;

  SpectMorph::WavSet    wset;
  std::string           wset_filename;
  std::string           wset_active_text;
  bool                  wset_edit;

  std::string           smset_dir;
  Gtk::ComboBoxText     smset_combobox;
  Gtk::VBox             index_vbox;
  Gtk::ToggleButton     show_position_button;
  Gtk::ToggleButton     show_analysis_button;
  Gtk::ToggleButton     show_frequency_grid_button;
  Gtk::Button           save_button;
  std::vector<float>    decoded_samples;

  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    ModelColumns()
    {
      add (col_note);
      add (col_channel);
      add (col_range);
      add (col_path);
      add (col_wave_nr);
    }
    Gtk::TreeModelColumn<int>           col_note;
    Gtk::TreeModelColumn<int>           col_channel;
    Gtk::TreeModelColumn<Glib::ustring> col_range;
    Gtk::TreeModelColumn<Glib::ustring> col_path;
    Gtk::TreeModelColumn<int>           col_wave_nr;
  };

  ModelColumns audio_chooser_cols;
  Glib::RefPtr<Gtk::ListStore>       ref_tree_model;
  Glib::RefPtr<Gtk::TreeSelection>   ref_tree_selection;
  Gtk::ScrolledWindow                tree_view_scrolled_window;
  Gtk::TreeView                      tree_view;

  Gtk::ToggleButton                  source_button;

  GslDataHandle                     *dhandle;
  Audio                             *audio;

  FFTParamWindow                     m_fft_param_window;
  DisplayParamWindow                 m_display_param_window;
  SpectrumWindow                     spectrum_window;
  SampleWindow                       sample_window;
  TimeFreqWindow                     time_freq_window;
  PlayerWindow                       player_window;
  LPCWindow                          lpc_window;
public:
  sigc::signal<void> signal_dhandle_changed;
  sigc::signal<void> signal_show_position_changed;
  sigc::signal<void> signal_show_analysis_changed;
  sigc::signal<void> signal_show_frequency_grid_changed;

  Navigator (const std::string& filename);

  void on_combo_changed();
  void on_selection_changed();
  void on_show_position_changed();
  void on_show_analysis_changed();
  void on_show_frequency_grid_changed();
  void on_save_clicked();
  void on_audio_edit();
  void on_next_sample();
  void on_view_time_freq();
  void on_view_sample();
  void on_view_spectrum();
  void on_view_fft_params();
  void on_view_display_params();
  void on_view_lpc();
  void on_view_player();
  bool on_delete_event (GdkEventAny* event);

  GslDataHandle *get_dhandle();
  Audio         *get_audio();
  bool           get_show_position();
  bool           get_show_analysis();
  bool           get_show_frequency_grid();
  bool           spectmorph_signal_active();

  FFTParamWindow     *fft_param_window();
  DisplayParamWindow *display_param_window();
#endif
};

}

#endif
