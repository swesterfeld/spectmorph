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

#ifndef SPECTMORPH_RANDOM_HH
#define SPECTMORPH_RANDOM_HH

#include <stdlib.h>
#include <glib.h>

namespace SpectMorph
{

class Random
{
  guint32 accu;

  // from rapicorn
  inline guint32
  quick_rand32 ()
  {
    accu = 1664525 * accu + 1013904223;
    return accu;
  }

public:
  Random();

  void set_seed (guint32 seed);

  inline double
  random_double_range (double begin, double end)
  {
    guint32 r = quick_rand32();

    const double scale = 1.0 / 4294967296.0;
    return r * scale * (end - begin) + begin;
  }

  inline guint32
  random_uint32()
  {
    return quick_rand32();
  }
};

}

#endif
