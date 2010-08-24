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

#include "smrandom.hh"

#include <string.h>

#include <glib.h>

using SpectMorph::Random;

Random::Random()
{
  memset (&buf, 0, sizeof (buf));
  for (size_t i = 0; i < sizeof (state); i++)
    state[i] = g_random_int();

  initstate_r (g_random_int(), state, sizeof (state), &buf);
}
