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
#include <bse/bseloader.h>
#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smfft.hh"

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

struct AnalysisParams
{
  double frame_size_ms;
  double frame_step_ms;
};

struct FFTResult
{
  vector<float> mags;
};

class TimeFreqView : public Gtk::DrawingArea
{
protected:
  vector<FFTResult> results;
  Glib::RefPtr<Gdk::Pixbuf> image;

public:
  TimeFreqView (const string& filename);

  void load (const string& filename);
  bool on_expose_event (GdkEventExpose* ev);
};

TimeFreqView::TimeFreqView (const string& filename)
{
  set_size_request (400, 400);

  load (filename);
}

struct Options
{
  string program_name;
} options;

void
TimeFreqView::load (const string& filename)
{
  BseErrorType error;
  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (filename.c_str(), &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }
  AnalysisParams analysis_params;
  analysis_params.frame_size_ms = 40;
  analysis_params.frame_step_ms = 10;

  size_t frame_size = analysis_params.frame_size_ms * gsl_data_handle_mix_freq (dhandle) / 1000.0;
  size_t block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  vector<float> block (block_size);
  vector<float> window (block_size);

  size_t zeropad = 4;
  size_t fft_size = block_size * zeropad;

  float *fft_in = FFT::new_array_float (fft_size);
  float *fft_out = FFT::new_array_float (fft_size);

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }


  double len_ms = gsl_data_handle_length (dhandle) * 1000.0 / gsl_data_handle_mix_freq (dhandle); 
  for (double pos_ms = analysis_params.frame_step_ms * 0.5 - analysis_params.frame_size_ms; pos_ms < len_ms; pos_ms += analysis_params.frame_step_ms)
    {
      int64 pos = pos_ms / 1000.0 * gsl_data_handle_mix_freq (dhandle);
      int64 todo = block.size(), offset = 0;
      const int64 n_values = gsl_data_handle_length (dhandle);

      while (todo)
        {
          int64 r = 0;
          if ((pos + offset) < 0)
            {
              r = 1;
              block[offset] = 0;
            }
          else if (pos + offset < n_values)
            {
              r = gsl_data_handle_read (dhandle, pos + offset, todo, &block[offset]);
            }
          if (r > 0)
            {
              offset += r;
              todo -= r;
            }
          else  // last block
            {
              while (todo)
                {
                  block[offset++] = 0;
                  todo--;
                }
            }
        }
      Bse::Block::mul (block_size, &block[0], &window[0]);
      for (size_t i = 0; i < fft_size; i++)
        {
          if (i < block_size)
            fft_in[i] = block[i];
          else
            fft_in[i] = 0;
        }
      FFT::fftar_float (fft_size, fft_in, fft_out);
      FFTResult result;
      fft_out[1] = 0; // special packing
      for (size_t i = 0; i < fft_size; i += 2)      
        {
          double re = fft_out[i];
          double im = fft_out[i + 1];

          result.mags.push_back (sqrt (re * re + im * im));
        }
      results.push_back (result);
    }
}

float
value_scale (float value)
{
  if (true)
    {
      double db = bse_db_from_factor (value, -200);
      if (db > -90)
        return db + 90;
      else
        return 0;
    }
  else
    return value;
}

bool
TimeFreqView::on_expose_event (GdkEventExpose *ev)
{
  if (!image)
    {
      image = Gdk::Pixbuf::create (Gdk::COLORSPACE_RGB, false, 8, results.size(), results[0].mags.size());

      float max_value = 0;
      for (vector<FFTResult>::const_iterator fi = results.begin(); fi != results.end(); fi++)
        {
          for (vector<float>::const_iterator mi = fi->mags.begin(); mi != fi->mags.end(); mi++)
            {
              max_value = max (max_value, value_scale (*mi));
            }
        }
      guchar *p = image->get_pixels();
      size_t  row_stride = image->get_rowstride();
      for (size_t frame = 0; frame < results.size(); frame++)
        {
          for (size_t m = 0; m < results[frame].mags.size(); m++)
            {
              double value = value_scale (results[frame].mags[m]);
              value /= max_value;
              int y = results[frame].mags.size() - 1 - m;
              p[row_stride * y + 0] = value * 255;
              p[row_stride * y + 1] = value * 255;
              p[row_stride * y + 2] = value * 255;
            }
          p += 3;
        }
      set_size_request (image->get_width(), image->get_height());
    }
  image->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, 0, 0, image->get_width(), image->get_height(),
                             // draw the whole image (from 0,0 to the full width,height) at 100,80 in the window
                             Gdk::RGB_DITHER_NONE, 0, 0);
  return true;
}

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  TimeFreqView        time_freq_view;

public:
  MainWindow (const string& filename);
};

#if 0
{
image->render_to_drawable(get_window(), get_style()->get_black_gc(),
0, 0, 100, 80, image->get_width(), image->get_height(), // draw the whole image (from 0,0 to the full width,height) at 100,80 in the window
Gdk::RGB_DITHER_NONE, 0, 0);
return true;
}
};

#endif

MainWindow::MainWindow (const string& filename) :
  time_freq_view (filename)
{
  set_border_width (10);
  add (scrolled_win);
  scrolled_win.add (time_freq_view);
  scrolled_win.show();
  time_freq_view.show();
}

int
main (int argc, char **argv)
{
  options.program_name = "sminspector";

  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  assert (argc == 2);

  MainWindow window (argv[1]);

  Gtk::Main::run (window);
}
