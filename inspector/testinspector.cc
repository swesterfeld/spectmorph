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
#include "smwavloader.hh"
#include "smcwt.hh"

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

void
show_progress (double p)
{
  static double old_progress = 0;
  if (p > old_progress + 0.01)
    {
      printf ("\rCWT: %.2f%%", p * 100);
      fflush (stdout);
      old_progress = p;
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  const double clocks_per_sec = 2500.0 * 1000 * 1000;

  if (argc == 2 && string (argv[1]) == "zoom")
    {
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
    }
  else if (argc == 3 && string (argv[1]) == "cwt")
    {
      CWT cwt;
      WavLoader *loader = WavLoader::load (argv[2]);
      if (!loader)
        return 1;

      vector<float> signal = loader->samples();
      vector< vector<float> > results;

      AnalysisParams params;
      params.cwt_freq_resolution = 25;
      params.cwt_time_resolution = 5;

      double start = gettime();
      cwt.signal_progress.connect (sigc::ptr_fun (show_progress));
      results = cwt.analyze (signal, params);
      double end = gettime();
      printf ("\n");
      cwt.make_png (results);

      printf ("per sample cost: %f ops\n", (end - start) * clocks_per_sec / (signal.size() * results.size()));
    }
  else
    {
      printf ("usage: testinspector zoom\n");
      printf ("or     testinspector cwt <somefile.wav>\n");
      return 1;
    }

  return 0;
}
