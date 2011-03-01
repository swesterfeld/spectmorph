/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

class MainWindow : public Gtk::Window
{
  Gtk::Button load_index_button;
  MorphPlan   morph_plan;

public:
  void
  on_button_clicked()
  {
    Gtk::FileChooserDialog dialog ("Select SpectMorph index file", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for (*this);

    // buttons
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // allow only .smindex files
    Gtk::FileFilter filter_smindex;
    filter_smindex.set_name ("SpectMorph index files");
    filter_smindex.add_pattern ("*.smindex");
    dialog.add_filter (filter_smindex);

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK)
      {
        morph_plan.load_index (dialog.get_filename());
      }
  }

  MainWindow() :
    load_index_button ("Load SpectMorph index file")
  {
    set_border_width (10);
    load_index_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWindow::on_button_clicked));
    add (load_index_button);

    show_all_children();
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  if (argc != 1)
    {
      printf ("usage: %s\n", argv[0]);
      exit (1);
    }

  // give parent our pid (for killing the gui)
  printf ("pid %d\n", getpid());
  fflush (stdout);

  MainWindow window;

  Gtk::Main::run (window);

  // let parent know that the user closed the gui window
  printf ("quit\n");
  fflush (stdout);
}
