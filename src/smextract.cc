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

#include <vector>
#include <stdio.h>
#include <bse/bsemain.h>
#include "stwafile.hh"
#include "frame.hh"

using Stw::Codec::Frame;
using std::vector;

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);

  Stw::Codec::AudioHandle audio;
  BseErrorType error = STWAFile::load (argv[1], audio);
  if (error)
    {
      fprintf (stderr, "can't load file: %s\n", argv[1]);
      exit (1);
    }

  int frame_size = audio->frame_size_ms * audio->mix_freq / 1000;

  if (strcmp (argv[2], "freq") == 0)
    {
      float freq_min = atof (argv[3]);
      float freq_max = atof (argv[4]);


      for (size_t i = 0; i < audio->contents.length(); i++)
        {
          Frame frame (audio->contents[i], frame_size);
          for (size_t n = 0; n < frame.freqs.size(); n++)
            {
              if (frame.freqs[n] > freq_min && frame.freqs[n] < freq_max)
                {
                  printf ("%zd %f %f\n", i, frame.freqs[n], frame.phases[n*2] /* optimistic */);
                }
            }
        }
    }
  else if (strcmp (argv[2], "spectrum") == 0)
    {
      int i = atoi (argv[3]);
      vector<double> spectrum;

      for (size_t n = 0; n < audio->contents[i]->original_fft.length(); n += 2)
        {
          double re = *(audio->contents[i]->original_fft.begin() + n);
          double im = *(audio->contents[i]->original_fft.begin() + n + 1);
          spectrum.push_back (sqrt (re * re + im * im));
        }
      for (size_t n = 0; n < spectrum.size(); n++)
        {
          double s = 0;
          for (size_t r = 0; r < 1; r++)
            {
              if (n + r < spectrum.size())
                s = std::max (s, spectrum[n + r]);
              if (r < n)
                s = std::max (s, spectrum[n - r]);
            }
          printf ("%f %f\n", n * 0.5 * audio->mix_freq / spectrum.size(), s);
        }
    }
  else if (strcmp (argv[2], "frame") == 0)
    {
      int i = atoi (argv[3]);
      size_t frame_size = audio->contents[i]->debug_samples.length();
      vector<double> sines (frame_size);
      for (size_t partial = 0; partial < audio->contents[i]->freqs.length(); partial++)
        {
          double f = *(audio->contents[i]->freqs.begin() + partial);
          double mag = *(audio->contents[i]->phases.begin() + 2 * partial);
          double phase = 0;
          for (size_t n = 0; n < frame_size; n++)
            {
              phase += f / audio->mix_freq * 2.0 * M_PI;
              sines[n] += sin (phase) * mag;
            }
        }
      for (size_t n = 0; n < audio->contents[i]->debug_samples.length(); n++)
        {
          double v = *(audio->contents[i]->debug_samples.begin() + n);
          printf ("%zd %f %f\n", n, v, sines[n]);
        }
    }
}
