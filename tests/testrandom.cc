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
#include <stdio.h>

int
main()
{
  SpectMorph::Random random;
  printf ("random (10000, 20000) = %f\n", random.random_double_range (10000, 20000));
  printf ("deterministic (-1 .. 1)\n");

  random.set_seed (42);
  for (int i = 0; i < 5; i++)
    printf ("%f\n", random.random_double_range (-1, 1));
}
