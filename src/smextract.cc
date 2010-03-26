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
#include <assert.h>

using Stw::Codec::Frame;
using std::vector;

double
float_vector_delta (const vector<float>& a, const vector<float>& b)
{
  assert (a.size() == b.size());

  double d = 0;
  for (size_t i = 0; i < a.size(); i++)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  return d;
}

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);

  SpectMorph::Audio audio;
  BseErrorType error = STWAFile::load (argv[1], audio);
  if (error)
    {
      fprintf (stderr, "can't load file: %s\n", argv[1]);
      exit (1);
    }

  int frame_size = audio.frame_size_ms * audio.mix_freq / 1000;

  if (strcmp (argv[2], "freq") == 0)
    {
      float freq_min = atof (argv[3]);
      float freq_max = atof (argv[4]);


      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          Frame frame (audio.contents[i], frame_size);
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

      for (size_t n = 0; n < audio.contents[i].original_fft.size(); n += 2)
        {
          double re = audio.contents[i].original_fft[n];
          double im = audio.contents[i].original_fft[n + 1];
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
          printf ("%f %f\n", n * 0.5 * audio.mix_freq / spectrum.size(), s);
        }
    }
  else if (strcmp (argv[2], "frame") == 0)
    {
      int i = atoi (argv[3]);
      size_t frame_size = audio.contents[i].debug_samples.size();
      vector<float> sines (frame_size);
      for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
        {
          double f = audio.contents[i].freqs[partial];
          double mag = audio.contents[i].phases[2 * partial];
          double phase = 0;
          double smag = 0, cmag = 0;
          for (size_t n = 0; n < frame_size; n++)
            {
              double v = audio.contents[i].debug_samples[n];
              phase += f / audio.mix_freq * 2.0 * M_PI;
              smag += sin (phase) * v;
              cmag += cos (phase) * v;
            }
          smag *= 2.0 / frame_size;
          cmag *= 2.0 / frame_size;
          printf ("%f %f %f %f\n", mag, smag, cmag, sqrt (smag * smag + cmag * cmag));
          phase = 0;
          vector<float> old_sines = sines;
          double delta = float_vector_delta (sines, audio.contents[i].debug_samples);
          for (size_t n = 0; n < frame_size; n++)
            {
              phase += f / audio.mix_freq * 2.0 * M_PI;
              sines[n] += sin (phase) * smag;
              sines[n] += cos (phase) * cmag;
            }
          double new_delta = float_vector_delta (sines, audio.contents[i].debug_samples);
          if (new_delta > delta)      // approximation is _not_ better
            sines = old_sines;
        }
      for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
        {
          double v = audio.contents[i].debug_samples[n];
          printf ("%zd %f %f\n", n, v, sines[n]);
        }
    }
}
