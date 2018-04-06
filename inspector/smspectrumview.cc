// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smspectrumview.hh"
#include "smnavigator.hh"

#include <QPainter>

using namespace SpectMorph;

using std::vector;
using std::max;

SpectrumView::SpectrumView (Navigator *navigator) :
  navigator (navigator)
{
  time_freq_view_ptr = NULL;
  hzoom = 1;
  vzoom = 1;
  update_size();
}

static float
value_scale (float value)
{
  double db = value;
  if (db > -96)
    return db + 96;
  else
    return 0;
}

void
SpectrumView::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.fillRect (rect(), QColor (255, 255, 255));

  float max_mag = -100;
  for (vector<float>::const_iterator mi = spectrum.mags.begin(); mi != spectrum.mags.end(); mi++)
    {
      max_mag   = max (*mi, max_mag);
    }
  const float max_value = value_scale (max_mag);

  const int width =  800 * hzoom;
  const int height = 600 * vzoom;

  painter.setPen (QPen (QColor (0, 0, 200), 2));
  if (time_freq_view_ptr->show_frequency_grid())
    {
      double fundamental_freq = time_freq_view_ptr->fundamental_freq();
      double mix_freq = time_freq_view_ptr->mix_freq();

      double pos;
      int partial = 1;
      do
        {
          pos = partial * fundamental_freq / (mix_freq / 2);
          partial++;

          painter.drawLine (pos * width, 0, pos * width, height);
        }
      while (pos < 1);
    }

  // draw helper lines @ 24, 48, 72
  painter.setPen (QPen (QColor (210, 210, 210), 2));
  for (int db = 24; db < 100; db += 24)
    {
      const int hy = height - value_scale (max_mag - db) / max_value * height;
      painter.drawLine (0, hy, width, hy);
    }

  // draw spectrum
  painter.setPen (QPen (QColor (200, 0, 0), 2));
  int last_x = 0, last_y = 0;
  for (size_t i = 0; i < spectrum.mags.size(); i++)
    {
      int x = double (i) / spectrum.mags.size() * width;
      int y = height - value_scale (spectrum.mags[i]) / max_value * height;
      if (i != 0)
        painter.drawLine (last_x, last_y, x, y);

      last_x = x;
      last_y = y;
    }
}

void
SpectrumView::set_spectrum_model (TimeFreqView *tfview)
{
  connect (tfview, SIGNAL (spectrum_changed()), this, SLOT (on_spectrum_changed()));
  time_freq_view_ptr = tfview;
}

void
SpectrumView::on_spectrum_changed()
{
  spectrum = time_freq_view_ptr->get_spectrum();
  audio_block = AudioBlock(); // reset

  Audio *audio = time_freq_view_ptr->audio();
  if (audio)
    {
      int frame = time_freq_view_ptr->position_frac() * audio->contents.size();
      int frame_count = audio->contents.size();

      if (frame >= 0 && frame < frame_count)
        {
          audio_block = audio->contents[frame];
        }
    }

  update();
}

void
SpectrumView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  update_size();
  update();
}

void
SpectrumView::update_size()
{
  resize (800 * hzoom, 600 * vzoom);
}

void
SpectrumView::on_display_params_changed()
{
  update();
}
