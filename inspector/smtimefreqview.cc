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

//  Glib::signal_timeout().connect (sigc::mem_fun (*this, &TimeFreqView::on_timeout), 400);
  FFTThread::the()->signal_result_available.connect (sigc::mem_fun (*this, &TimeFreqView::on_timeout));
}

void
TimeFreqView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
TimeFreqView::on_timeout()
{
  if (FFTThread::the()->get_result (image))
    force_redraw();
}

void
TimeFreqView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;
  zimage.clear();

  force_redraw();
}

void
TimeFreqView::set_position (int new_position)
{
  position = new_position;
  zimage.clear();

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
  results.clear();

  if (dhandle) // NULL dhandle means user opened a new instrument but did not select anything yet
    {
      FFTThread::the()->compute_image (image, dhandle, analysis_params);
    }
  this->audio = audio;


  force_redraw();
  signal_spectrum_changed();
}

int
TimeFreqView::get_frames()
{
  return results.size();
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
  double scaled_hzoom = 400 * hzoom / MAX (image.get_width(), 1);
  double scaled_vzoom = 2000 * vzoom / MAX (image.get_height(), 1);
  set_size_request (image.get_width() * scaled_hzoom, image.get_height() * scaled_vzoom);
  zimage = zoom_rect (image, ev->area.x, ev->area.y, ev->area.width, ev->area.height, scaled_hzoom, scaled_vzoom, position);
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, ev->area.x, ev->area.y,
                              zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window && audio && show_analysis)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      cr->set_line_width (1.0);

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      cr->clip();

      // red:
      cr->set_source_rgb (1.0, 0.0, 0.0);

      int width = image.get_width() * scaled_hzoom, height = image.get_height() * scaled_vzoom;

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
  return true;
}

FFTResult
TimeFreqView::get_spectrum()
{
  if (position >= 0 && position < int (results.size()))
    return results[position];

  return FFTResult();
}

void
TimeFreqView::set_show_analysis (bool new_show_analysis)
{
  show_analysis = new_show_analysis;

  force_redraw();
}
