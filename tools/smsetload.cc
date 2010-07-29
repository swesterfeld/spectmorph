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

#include <stdio.h>
#include <smwavset.hh>
#include <smaudio.hh>

using namespace SpectMorph;
using std::vector;

int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      printf ("usage: %s <smset-filename>\n", argv[0]);
      return 1;
    }

  WavSet wset;
  if (wset.load (argv[1]))
    {
      printf ("cannot load smset: %s\n", argv[1]);
      return 1;
    }

  for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
    {
      SpectMorph::Audio *audio = new SpectMorph::Audio;
      int error = audio->load (wi->path, AUDIO_SKIP_DEBUG);
      if (error)
        {
          printf ("cannot load sm-wave: %s\n", wi->path.c_str());
          return 1;
        }
    }

  return 0;
}
