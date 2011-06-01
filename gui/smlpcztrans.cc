/*
 * Copyright (C) 2011 Stefan Westerfeld
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
#include <bse/bsemathsignal.h>
#include <complex>
#include <vector>
#include "smlpcztrans.hh"

using namespace SpectMorph;

using std::vector;
using std::complex;

GdkPixbuf *
SpectMorph::lpc_z_transform (const vector<double>& a, const vector< complex<double> >& roots)
{
  const size_t width = 1000, height = 1000;

  GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, /* has_alpha */ false, 8, width, height);
  uint row_stride = gdk_pixbuf_get_rowstride (pixbuf);

  for (size_t y = 0; y < width; y++)
    {
      for (size_t x = 0; x < width; x++)
        {
          double re = double (x * 2) / width - 1;
          double im = double (y * 2) / height - 1;
          guchar *p = gdk_pixbuf_get_pixels (pixbuf) + 3 * x + y * row_stride;

          complex<double> z (re, im);
          complex<double> acc = -1;

          for (int j = 0; j < int (a.size()); j++)
            acc += pow (z, -(j + 1)) * a[j];
          double value = 1 / abs (acc);
          double db = bse_db_from_factor (value, -200);
          db += 150;

          int idb = CLAMP (db, 0, 255);
          p[0] = idb;
          p[1] = idb;
          p[2] = idb;
        }
    }
  for (size_t i = 0; i < roots.size(); i++)
    {
      int x = (roots[i].real() + 1) * width * 0.5;
      int y = (roots[i].imag() + 1) * height * 0.5;
      if (x >= 3 && x < int (width - 3) && y >= 3 && y < int (height - 3))
        {
          guchar *p = gdk_pixbuf_get_pixels (pixbuf) + 3 * x + y * row_stride;
          int down_right = row_stride + 3;
          int down_left = row_stride - 3;
          for (int delta = -3; delta <= 3; delta++)
            {
              p[0 + delta * down_right] = p[0 + delta * down_left] = 255;
              p[1 + delta * down_right] = p[1 + delta * down_left] = 0;
              p[2 + delta * down_right] = p[2 + delta * down_left] = 0;
            }
        }
    }
  return pixbuf;
}


