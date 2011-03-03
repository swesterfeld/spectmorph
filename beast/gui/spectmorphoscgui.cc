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
#include "smmorphsource.hh"
#include "smmorphplanview.hh"

using namespace SpectMorph;

using std::string;

class MainWindow : public Gtk::Window
{
  Gtk::Button   load_index_button;
  Gtk::Button   add_operator_button;
  Gtk::VBox     button_vbox;
  MorphPlan     morph_plan;
  MorphPlanView morph_plan_view;

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
  void
  on_add_operator_button_clicked()
  {
    morph_plan.add_operator (new MorphSource());
  }
  MainWindow() :
    load_index_button ("Load SpectMorph index file"),
    add_operator_button ("Add Operator"),
    morph_plan_view (&morph_plan)
  {
    set_border_width (10);
    load_index_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWindow::on_button_clicked));
    add_operator_button.signal_clicked().connect (sigc::mem_fun (*this, &MainWindow::on_add_operator_button_clicked));
    button_vbox.add (load_index_button);
    button_vbox.add (add_operator_button);
    button_vbox.add (morph_plan_view);
    add (button_vbox);

    show_all_children();
  }
  void
  set_plan_str (const string& plan_str)
  {
    morph_plan.set_plan_str (plan_str);
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

  // read initial plan
  string plan_str;
  int ch;
  while ((ch = fgetc (stdin)) != '\n')
    plan_str += (char) ch;

  // give parent our pid (for killing the gui)
  printf ("pid %d\n", getpid());
  fflush (stdout);

  MainWindow window;

  window.set_plan_str (plan_str);

  Gtk::Main::run (window);

  // let parent know that the user closed the gui window
  printf ("quit\n");
  fflush (stdout);
}
