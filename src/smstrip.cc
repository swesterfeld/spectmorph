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
#include "smaudio.hh"

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      printf ("usage: smstrip <filename.sm> [...]\n");
      exit (1);
    }
  for (int n = 1; n < argc; n++)
    {
      SpectMorph::Audio audio;
      BseErrorType error = audio.load (argv[n], SpectMorph::AUDIO_SKIP_DEBUG);
      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[n], bse_error_blurb (error));
          exit (1);
        }
      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          audio.contents[i].debug_samples.clear();
          audio.contents[i].original_fft.clear();
        }
      audio.save (argv[n]);
    }
}
