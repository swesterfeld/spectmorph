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
#include "smsampleview.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

class MainWindow : public Gtk::Window
{
  SampleView view;

public:
  MainWindow();
};

MainWindow::MainWindow()
{
  add (view);
  show_all_children();
}

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

  MainWindow main_window;
  Gtk::Main::run (main_window);

  return 0;
}
