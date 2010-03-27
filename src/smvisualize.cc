/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <bse/bse.h>
#include <bse/bsemain.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "smaudio.hh"
#include "stwafile.hh"

using std::vector;

bool db_mode = false;
const int BORDER_PIXELS = 10;

float
value_scale (float rvalue, float ivalue = 0)
{
  double value = sqrt (rvalue * rvalue + ivalue * ivalue);

  if (db_mode)
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

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);
  if (argc == 4 && strcmp (argv[3], "db") == 0)
    {
      argc--;
      db_mode = true;
    }
  if (argc != 3)
    {
      fprintf (stderr, "usage: %s <stwa-file> <png-file> [ db ]\n", argv[0]);
      exit (1);
    }

  SpectMorph::Audio audio;
  BseErrorType file_error = STWAFile::load (argv[1], audio);
  if (file_error)
    {
      fprintf (stderr, "can't read input file: %s\n", bse_error_blurb (file_error));
      exit (1);
    }

  const double mix_freq = audio.mix_freq;
  const uint64 block_size = audio.contents[0].original_fft.size() - 2;

  GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, /* has_alpha */ false, 8,
                      audio.contents.size() * 2 + BORDER_PIXELS * 2, (block_size + 2) / 2);

  double max_value = 0.0;
  // compute magnitudes from FFT data and figure out max peak
  for (size_t n = 0; n < audio.contents.size(); n++)
    {
      vector<float>& original_fft = audio.contents[n].original_fft;
      for (size_t d = 0; d < original_fft.size(); d += 2)
        {
          double re = original_fft[d];
          double im = original_fft[d + 1];
          double value = value_scale (re, im);
	  max_value = std::max (value, max_value);
          original_fft[d] = value;     // magnitude
          original_fft[d + 1] = 0;     // phase - not computed for now
        }
    }

  guchar *p = gdk_pixbuf_get_pixels (pixbuf) + 3 * BORDER_PIXELS;
  uint row_stride = gdk_pixbuf_get_rowstride (pixbuf);
  for (size_t n = 0; n < BORDER_PIXELS; n++)
    {
      for (size_t y = 0; y < block_size / 2; y++)
        {
          for (int i = 0; i < 3 * BORDER_PIXELS; i++)
            {
              // draw a black border left of the analysis
              p[-30 + row_stride * y + i] = 0;

              // draw a black border right of the analysis
              p[row_stride * y + i + 6 * audio.contents.size()] = 0;
            }
        }
    }
  for (size_t n = 0; n < audio.contents.size(); n++)
    {
      const vector<float>& original_fft = audio.contents[n].original_fft;
      for (size_t d = 0; d < original_fft.size(); d += 2)
        {
          double value = original_fft[d];
          float f = value / max_value;
          int y = (block_size/2) - (d/2);
          p[row_stride * y] = f * 255;
          p[row_stride * y + 1] = f * 255;
          p[row_stride * y + 2] = f * 255;
          p[row_stride * y + 3] = f * 255;
          p[row_stride * y + 4] = f * 255;
          p[row_stride * y + 5] = f * 255;
#if 0	  /* delta phases not drawn for now */
	  float delta_p = 0.5;
	  if (f > 0.1)
	    delta_p = *(meaning.begin() + d + 1) / (f > 0 ? f : 1) + 0.5;

	  p[row_stride * (d/2) + 6 * audio->contents.size()] = delta_p * 255;
	  p[row_stride * (d/2) + 6 * audio->contents.size() + 1] = delta_p * 255;
	  p[row_stride * (d/2) + 6 * audio->contents.size() + 2] = delta_p * 255;
	  p[row_stride * (d/2) + 6 * audio->contents.size() + 3] = delta_p * 255;
	  p[row_stride * (d/2) + 6 * audio->contents.size() + 4] = delta_p * 255;
	  p[row_stride * (d/2) + 6 * audio->contents.size() + 5] = delta_p * 255;
#endif
	}
      vector<float>::const_iterator fi;
      for (fi = audio.contents[n].freqs.begin(); fi != audio.contents[n].freqs.end(); fi++)
        {
          double freq = *fi;
          int d_2 = CLAMP (freq / mix_freq * block_size, 0, block_size);
          int y = block_size / 2 - d_2;
          p[row_stride * y] = 255;  /* R */
          p[row_stride * y + 1] = 0;  /* R */
          p[row_stride * y + 2] = 0;  /* R */
          p[row_stride * y + 3] = 255;  /* R */
          p[row_stride * y + 4] = 0;  /* R */
          p[row_stride * y + 5] = 0;  /* R */
          
        }
      p += 6;
    }

  GError *error = 0;
  gdk_pixbuf_save (pixbuf, argv[2], "png", &error, "compression", "0", NULL);
}
