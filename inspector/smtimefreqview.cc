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

#include "smtimefreqview.hh"
#include "smfft.hh"

#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

namespace {
struct Options {
  string program_name;
} options;
}

TimeFreqView::TimeFreqView()
{
  set_size_request (400, 400);
  hzoom = 1;
  vzoom = 1;
  position = -1;
}

void
TimeFreqView::set_hzoom (double new_hzoom)
{
  hzoom = new_hzoom;
  zimage.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
TimeFreqView::set_vzoom (double new_vzoom)
{
  vzoom = new_vzoom;
  zimage.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
TimeFreqView::set_position (int new_position)
{
  position = new_position;
  zimage.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
  signal_spectrum_changed();
}

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
  load (dhandle, filename);
}

void
TimeFreqView::load (GslDataHandle *dhandle, const string& filename)
{
  results.clear();

  BseErrorType error = gsl_data_handle_open (dhandle);
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
  image.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

int
TimeFreqView::get_frames()
{
  return results.size();
}

static float
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

#define FRAC_SHIFT       12
#define FRAC_FACTOR      (1 << FRAC_SHIFT)
#define FRAC_HALF_FACTOR (1 << (FRAC_SHIFT - 1))

Glib::RefPtr<Gdk::Pixbuf>
TimeFreqView::zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth, double hzoom, double vzoom, int position)
{
  Glib::RefPtr<Gdk::Pixbuf> zimage = Gdk::Pixbuf::create (Gdk::COLORSPACE_RGB, false, 8, destw, desth);
  int hzoom_inv_frac = FRAC_FACTOR / hzoom;
  int vzoom_inv_frac = FRAC_FACTOR / vzoom;

  guchar *p = zimage->get_pixels();
  size_t  row_stride = zimage->get_rowstride();

  guchar *pixel_array_p          = image.get_pixels();
  size_t  pixel_array_row_stride = image.get_rowstride();

  for (int y = 0; y < desth; y++)
    {
      guchar *pp = (p + row_stride * y);
      size_t outy = ((y + desty) * vzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;

      for (int x = 0; x < destw; x++)
        {
          size_t outx = ((x + destx) * hzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;
          int color;
          if (outx < image.get_width() && outy < image.get_height())
            color = pixel_array_p[pixel_array_row_stride * outy + outx];
          else
            color = 0;
          pp[0] = color;
          pp[1] = color;
          pp[2] = color;
          if (outx == position)
            pp[0] = 255;
          pp += 3;
        }
    }
  return zimage;
}

bool
TimeFreqView::on_expose_event (GdkEventExpose *ev)
{
  if (image.empty())
    {
      size_t height = 0;
      if (!results.empty())
        height = results[0].mags.size();

      image.resize (results.size(), height);

      float max_value = 0;
      for (vector<FFTResult>::const_iterator fi = results.begin(); fi != results.end(); fi++)
        {
          for (vector<float>::const_iterator mi = fi->mags.begin(); mi != fi->mags.end(); mi++)
            {
              max_value = max (max_value, value_scale (*mi));
            }
        }
      guchar *p = image.get_pixels();
      size_t  row_stride = image.get_rowstride();
      for (size_t frame = 0; frame < results.size(); frame++)
        {
          for (size_t m = 0; m < results[frame].mags.size(); m++)
            {
              double value = value_scale (results[frame].mags[m]);
              value /= max_value;
              int y = results[frame].mags.size() - 1 - m;
              p[row_stride * y] = value * 255;
            }
          p++;
        }
    }
  /*
  if (!zimage)
    {
      zimage = image->scale_simple (image->get_width() * hzoom, image->get_height() * vzoom, Gdk::INTERP_BILINEAR);
      set_size_request (zimage->get_width(), zimage->get_height());
    }
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, 0, 0, zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);
  */
  set_size_request (image.get_width() * hzoom, image.get_height() * vzoom);
  zimage = zoom_rect (image, ev->area.x, ev->area.y, ev->area.width, ev->area.height, hzoom, vzoom, position);
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, ev->area.x, ev->area.y,
                              zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);
  return true;
}

FFTResult
TimeFreqView::get_spectrum()
{
  if (position >= 0 && position < int (results.size()))
    return results[position];

  return FFTResult();
}


