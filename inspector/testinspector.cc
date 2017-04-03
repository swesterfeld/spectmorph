// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
#include <sys/time.h>
#include <stdio.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smtimefreqwindow.hh"
#include "smwavloader.hh"
#include "smcwt.hh"
#include "smsampleview.hh"

using std::vector;
using std::string;
using std::max;
using std::pair;
using std::make_pair;

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

#define DEBUG 0

struct DummyPainter
{
  vector<int> v;
  void drawLine (int x, int y, int ex, int ey)
  {
    if (DEBUG)
      printf ("drawLine (%d, %d, %d, %d);\n", x, y, ex, ey);
    v.push_back (x);
    v.push_back (y);
    v.push_back (ex);
    v.push_back (ey);
  }
} dummy_painter;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  const double clocks_per_sec = 2500.0 * 1000 * 1000;

  if (argc == 2 && string (argv[1]) == "zoom")
    {
      const unsigned int runs = 1000;

      PixelArray image;
      QImage zimage;
      double hzoom = 1.3, vzoom = 1.5;
      image.resize (1024, 1024);

      // warmup run:
      zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1, -96, 0);

      // timed runs:
      double start = gettime();
      for (unsigned int i = 0; i < runs; i++)
        zimage = TimeFreqView::zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom, -1, -96, 0);
      double end = gettime();

      printf ("zoom_rect: %f clocks/pixel\n", clocks_per_sec * (end - start) / (300 * 300) / runs);
      printf ("zoom_rect: %f Mpixel/s\n", 1.0 / ((end - start) / (300 * 300) / runs) / 1000 / 1000);
    }
  else if (argc == 2 && string (argv[1]) == "sample")
    {
      vector<float> signal;
      for (size_t i = 0; i < 1000000; i++)
        signal.push_back (g_random_double());

      int width = 1000;
      int height = 1000;

      QRect rect (0, 0, width, height);

      const double vz = height / 2;
      const double hz = double (width) / signal.size();

      const unsigned int runs = 100;
      double start = gettime();
      for (unsigned int i = 0; i < runs; i++)
        SampleView::draw_signal (signal, dummy_painter, rect, height, vz, hz);
      double end = gettime();

      printf ("draw_signal: %f clocks/value\n", clocks_per_sec * (end - start) / (signal.size()) / runs);
    }
  else if (argc == 3 && string (argv[1]) == "cwt")
    {
      CWT cwt;
      WavData wav_data;
      if (!wav_data.load (argv[2]))
        {
          fprintf (stderr, "load file %s failed: %s\n", argv[2], wav_data.error_blurb());
          return 1;
        }

      vector<float> signal = wav_data.samples();
      vector< vector<float> > results;

      AnalysisParams params;
      params.cwt_freq_resolution = 25;
      params.cwt_time_resolution = 5;

      double start = gettime();
      cwt.connect (&cwt, &CWT::signal_progress, show_progress);
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
      printf ("or     testinspector sample\n");
      return 1;
    }
  return 0;
}
