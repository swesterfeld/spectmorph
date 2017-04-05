// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimefreqview.hh"
#include "smfft.hh"
#include "smmath.hh"

#include <stdio.h>

#include <QPainter>
#include <QPaintEvent>
#include <QFileDialog>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

void
TimeFreqView::load (const WavData *wav_data, const string& filename, Audio *audio, const AnalysisParams& analysis_params)
{
  if (wav_data) // NULL wav_data means user opened a new instrument but did not select anything yet
    FFTThread::the()->compute_image (*wav_data, analysis_params);

  m_audio = audio;
}

#if 0
namespace {
struct Options {
  string program_name;
} options;
}
#endif

TimeFreqView::TimeFreqView()
{
  resize (400, 400);
  hzoom = 1;
  vzoom = 1;
  position = -1;
  m_audio = NULL;
  show_analysis = false;
  m_show_frequency_grid = false;

  display_min_db = -96;
  display_boost = 0;

  connect (FFTThread::the(), SIGNAL (result_available()), this, SLOT (on_result_available()));
}

void
TimeFreqView::scale_zoom (double *scaled_hzoom, double *scaled_vzoom)
{
  *scaled_hzoom = 800 * hzoom / MAX (image.get_width(), 1);
  *scaled_vzoom = 2000 * vzoom / MAX (image.get_height(), 1);
}

void
TimeFreqView::update_size()
{
  // resize widget according to zoom if necessary
  double scaled_hzoom, scaled_vzoom;
  scale_zoom (&scaled_hzoom, &scaled_vzoom);

  int new_width = image.get_width() * scaled_hzoom;
  int new_height = image.get_height() * scaled_vzoom;
  setFixedSize (new_width, new_height);
}

void
TimeFreqView::on_result_available()
{
  if (FFTThread::the()->get_result (image))
    {
      update_size();
      update();

      Q_EMIT spectrum_changed();
    }
  Q_EMIT progress_changed();
}

void
TimeFreqView::on_export()
{
  QString fileName = QFileDialog::getSaveFileName (this, "Save File");

  if (!fileName.isEmpty())
    {
      std::string fn = fileName.toLocal8Bit().data();
      FILE *out = fopen (fn.c_str(), "w");
      for (size_t y = 0; y < image.get_height(); y++)
        {
          const int *pixels = image.get_pixels() + y * image.get_rowstride();
          for (size_t x = 0; x < image.get_width(); x++)
            fprintf (out, "%d ", *pixels++);
          fprintf (out, "\n");
        }
      fclose (out);
    }
}

void
TimeFreqView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  update_size();
  update();
}

void
TimeFreqView::set_position (int new_position)
{
  position = new_position;

  update();
  Q_EMIT spectrum_changed();
}

#if 0
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
#endif

int
TimeFreqView::get_frames()
{
  return image.get_width();
}

QImage
TimeFreqView::zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth, double hzoom, double vzoom,
                         int position, double display_min_db, double display_boost)
{
  QImage zqimage (destw, desth, QImage::Format_RGB32);
  const double hzoom_inv = 1.0 / hzoom;
  const double vzoom_inv = 1.0 / vzoom;

  int    *pixel_array_p          = image.get_pixels();
  size_t  pixel_array_row_stride = image.get_rowstride();

  double abs_min_db = fabs (display_min_db);

  const int pixel_add   = 256 * (abs_min_db + display_boost);   // 8 bits fixed point
  const int pixel_scale = 64 * 255 / abs_min_db;                // 6 bits fixed point

  int scaledx[destw];
  for (int x = 0; x < destw; x++)
    {
      scaledx[x] = sm_round_positive ((x + destx) * hzoom_inv);
    }

  for (int y = 0; y < desth; y++)
    {
      size_t outy = sm_round_positive ((y + desty) * vzoom_inv);
      QRgb *rgb = static_cast<QRgb*> ((void *)zqimage.scanLine (y));

      for (int x = 0; x < destw; x++)
        {
          size_t outx = scaledx[x];
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

          if (int (outx) == position)
            *rgb++ = qRgb (255, color, color);
          else
            *rgb++ = qRgb (color, color, color);
        }
    }
  return zqimage;
}

void
TimeFreqView::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);

  double scaled_hzoom, scaled_vzoom;
  scale_zoom (&scaled_hzoom, &scaled_vzoom);

  QImage zqimage;
  zqimage = zoom_rect (image, event->rect().x(), event->rect().y(), event->rect().width(), event->rect().height(),
                       scaled_hzoom, scaled_vzoom, position,
                       display_min_db, display_boost);

  QPoint origin (event->rect().x(), event->rect().y());
  painter.drawImage (origin, zqimage);

  const int width = image.get_width() * scaled_hzoom, height = image.get_height() * scaled_vzoom;
  if (show_analysis)
    {
      // red:
      painter.setPen (QColor (255, 0, 0));

      const double size = 3;
      for (size_t i = 0; i < m_audio->contents.size(); i++)
        {
          double posx = width * double (i) / m_audio->contents.size();
          if (posx > event->rect().x() - 10 && posx < event->rect().width() + event->rect().x() + 10)
            {
              const AudioBlock& ab = m_audio->contents[i];
              for (size_t f = 0; f < ab.freqs.size(); f++)
                {
                  double posy = height - height * ab.freqs_f (f) * m_audio->fundamental_freq / (m_audio->mix_freq / 2);
                  painter.drawLine (posx - size, posy - size, posx + size, posy + size);
                  painter.drawLine (posx - size, posy + size, posx + size, posy - size);
                }
            }
        }
    }
  if (m_show_frequency_grid)
    {
      painter.setPen (QPen (QColor (128, 128, 255), 2));
      for (int partial = 1; ; partial++)
        {
          double posy = height - height * partial * m_audio->fundamental_freq / (m_audio->mix_freq / 2);
          if (posy < 0)
            break;

          painter.drawLine (0, posy, width, posy);
        }
    }
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

  update();
}

void
TimeFreqView::set_show_frequency_grid (bool new_show_frequency_grid)
{
  m_show_frequency_grid = new_show_frequency_grid;

  update();
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
  update();
}

double
TimeFreqView::fundamental_freq()
{
  if (m_audio)
    return m_audio->fundamental_freq;
  else
    return 0;
}

double
TimeFreqView::mix_freq()
{
  if (m_audio)
    return m_audio->mix_freq;
  else
    return 0;
}

bool
TimeFreqView::show_frequency_grid()
{
  return m_show_frequency_grid;
}

Audio *
TimeFreqView::audio()
{
  return m_audio;
}

double
TimeFreqView::position_frac()
{
  return position / double (image.get_width());
}
