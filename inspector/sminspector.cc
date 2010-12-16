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

#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smmainwindow.hh"

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  assert (argc == 2 || argc == 3);
  if (argc == 3)
    {
      if (string (argv[2]) == "perf")
        {
          const double clocks_per_sec = 2500.0 * 1000 * 1000;
          const unsigned int runs = 10;

          PixelArray image;
          Glib::RefPtr<Gdk::Pixbuf> zimage;
          double hzoom = 1.3, vzoom = 1.5;
          image.resize (1024, 1024);

          // warmup run:
          zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1);

          // timed runs:
          double start = gettime();
          for (unsigned int i = 0; i < runs; i++)
            zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1);
          double end = gettime();

          printf ("zoom_rect: %f clocks/pixel\n", clocks_per_sec * (end - start) / (300 * 300) / runs);
          printf ("zoom_rect: %f Mpixel/s\n", 1.0 / ((end - start) / (300 * 300) / runs) / 1000 / 1000);

          return 0;
        }
      else
        {
          assert (false);
        }
    }

  MainWindow window (argv[1]);

  Gtk::Main::run (window);
}
