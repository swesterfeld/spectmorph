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
  audio = NULL;
  show_analysis = false;
  m_show_frequency_grid = false;

  old_height = -1;
  old_width = -1;

  FFTThread::the()->signal_result_available.connect (sigc::mem_fun (*this, &TimeFreqView::on_result_available));
}

void
TimeFreqView::scale_zoom (double *scaled_hzoom, double *scaled_vzoom)
{
  *scaled_hzoom = 400 * hzoom / MAX (image.get_width(), 1);
  *scaled_vzoom = 2000 * vzoom / MAX (image.get_height(), 1);
}

void
TimeFreqView::force_redraw()
{
  // resize widget according to zoom if necessary
  double scaled_hzoom, scaled_vzoom;
  scale_zoom (&scaled_hzoom, &scaled_vzoom);

  int new_width = image.get_width() * scaled_hzoom;
  int new_height = image.get_height() * scaled_vzoom;
  if (new_width != old_width || new_height != old_height)
    {
      set_size_request (new_width, new_height);
      signal_resized (old_width, old_height, new_width, new_height);

      old_height = new_height;
      old_width = new_width;
    }

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
TimeFreqView::on_result_available()
{
  if (FFTThread::the()->get_result (image))
    {
      force_redraw();
      signal_spectrum_changed();
    }
  signal_progress_changed();
}

void
TimeFreqView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}

void
TimeFreqView::set_position (int new_position)
{
  position = new_position;

  force_redraw();
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
  AnalysisParams params;
  params.transform_type = SM_TRANSFORM_FFT;
  params.frame_size_ms = 40;
  params.frame_step_ms = 10;
  load (dhandle, filename, NULL, params);
}

void
TimeFreqView::load (GslDataHandle *dhandle, const string& filename, Audio *audio, const AnalysisParams& analysis_params)
{
  if (dhandle) // NULL dhandle means user opened a new instrument but did not select anything yet
    FFTThread::the()->compute_image (image, dhandle, analysis_params);

  this->audio = audio;
}

int
TimeFreqView::get_frames()
{
  return image.get_width();
}

#define FRAC_SHIFT       12
#define FRAC_FACTOR      (1 << FRAC_SHIFT)
#define FRAC_HALF_FACTOR (1 << (FRAC_SHIFT - 1))

Glib::RefPtr<Gdk::Pixbuf>
TimeFreqView::zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth, double hzoom, double vzoom,
                         int position, double display_min_db, double display_boost)
{
  Glib::RefPtr<Gdk::Pixbuf> zimage = Gdk::Pixbuf::create (Gdk::COLORSPACE_RGB, false, 8, destw, desth);
  int hzoom_inv_frac = FRAC_FACTOR / hzoom;
  int vzoom_inv_frac = FRAC_FACTOR / vzoom;

  guchar *p = zimage->get_pixels();
  size_t  row_stride = zimage->get_rowstride();

  int    *pixel_array_p          = image.get_pixels();
  size_t  pixel_array_row_stride = image.get_rowstride();

  double abs_min_db = fabs (display_min_db);

  const int pixel_add   = 256 * (abs_min_db + display_boost);   // 8 bits fixed point
  const int pixel_scale = 64 * 255 / abs_min_db;                // 6 bits fixed point

  for (int y = 0; y < desth; y++)
    {
      guchar *pp = (p + row_stride * y);
      size_t outy = ((y + desty) * vzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;

      for (int x = 0; x < destw; x++)
        {
          size_t outx = ((x + destx) * hzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;
          int color;
          if (outx < image.get_width() && outy < image.get_height())
            {
              color = (pixel_array_p[pixel_array_row_stride * outy + outx] + pixel_add) * pixel_scale;
              if (color < 0)
                color = 0;
              color >>= 14;                                     // 8 + 6 bits fixed point
              if (color > 255)
                color = 255;
            }
          else
            color = 0;
          pp[0] = color;
          pp[1] = color;
          pp[2] = color;
          if (int (outx) == position)
            pp[0] = 255;
          pp += 3;
        }
    }
  return zimage;
}

bool
TimeFreqView::on_expose_event (GdkEventExpose *ev)
{
  double scaled_hzoom, scaled_vzoom;
  scale_zoom (&scaled_hzoom, &scaled_vzoom);

  // draw contents
  Glib::RefPtr<Gdk::Pixbuf> zimage;
  zimage = zoom_rect (image, ev->area.x, ev->area.y, ev->area.width, ev->area.height, scaled_hzoom, scaled_vzoom, position,
                      display_min_db, display_boost);
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, ev->area.x, ev->area.y,
                              zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window && audio)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      cr->set_line_width (1.0);

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      cr->clip();

      int width = image.get_width() * scaled_hzoom, height = image.get_height() * scaled_vzoom;

      if (show_analysis)
        {
          // red:
          cr->set_source_rgb (1.0, 0.0, 0.0);

          const double size = 3;
          for (size_t i = 0; i < audio->contents.size(); i++)
            {
              double posx = width * double (i) / audio->contents.size();
              if (posx > ev->area.x - 10 && posx < ev->area.width + ev->area.x + 10)
                {
                  const AudioBlock& ab = audio->contents[i];
                  for (size_t f = 0; f < ab.freqs.size(); f++)
                    {
                      double posy = height - height * ab.freqs[f] / (audio->mix_freq / 2);
                      cr->move_to (posx - size, posy - size);
                      cr->line_to (posx + size, posy + size);
                      cr->move_to (posx - size, posy + size);
                      cr->line_to (posx + size, posy - size);
                    }
                }
            }
          cr->stroke();
        }
      if (m_show_frequency_grid)
        {
          cr->set_line_width (2.0);
          cr->set_source_rgb (0.5, 0.5, 1.0);

          for (int partial = 1; ; partial++)
            {
              double posy = height - height * partial * audio->fundamental_freq / (audio->mix_freq / 2);
              if (posy < 0)
                break;

              cr->move_to (0, posy);
              cr->line_to (width, posy);
            }
          cr->stroke();
        }
    }
  return true;
}

FFTResult
TimeFreqView::get_spectrum()
{
  if (position >= 0 && position < int (image.get_width()))
    {
      FFTResult result;
      result.mags.resize (image.get_height());

      int *ip = image.get_pixels() + position;
      for (size_t y = 0; y < image.get_height(); y++)
        {
          // we insert data backwards, so that low frequencies are it the beginning of the result vector
          result.mags[image.get_height() - y - 1] = *ip * (1.0 / 256);  // undo 8 bits fixed point
          ip += image.get_rowstride();
        }
      return result;
    }

  return FFTResult();
}

void
TimeFreqView::set_show_analysis (bool new_show_analysis)
{
  show_analysis = new_show_analysis;

  force_redraw();
}

void
TimeFreqView::set_show_frequency_grid (bool new_show_frequency_grid)
{
  m_show_frequency_grid = new_show_frequency_grid;

  force_redraw();
}

double
TimeFreqView::get_progress()
{
  return fft_thread.get_progress();
}

void
TimeFreqView::set_display_params (double min_db, double boost)
{
  display_min_db = min_db;
  display_boost = boost;
  force_redraw();
}

double
TimeFreqView::fundamental_freq()
{
  if (audio)
    return audio->fundamental_freq;
  else
    return 0;
}

double
TimeFreqView::mix_freq()
{
  if (audio)
    return audio->mix_freq;
  else
    return 0;
}

bool
TimeFreqView::show_frequency_grid()
{
  return m_show_frequency_grid;
}
