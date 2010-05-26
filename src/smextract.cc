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
#include "smafile.hh"
#include "smframe.hh"
#include <assert.h>
#include <bse/bsemathsignal.h>
#include <bse/gslfft.h>

using SpectMorph::Frame;
using SpectMorph::AudioBlock;
using std::vector;
using std::min;

double
vector_delta (const vector<double>& a, const vector<double>& b)
{
  assert (a.size() == b.size());

  double d = 0;
  for (size_t i = 0; i < a.size(); i++)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  return d;
}

/// @cond
// not yet part of the public API
struct TransientModel
{
  double start_volume;
  double end_volume;
  int start;
  int end;
};
/// @endcond

void
transient_scale (const TransientModel& m, const vector<double>& in, vector<double>& out)
{
  out.resize (in.size());

  for (size_t n = 0; n < in.size(); n++)
    {
      if (n <= m.start)
        {
          out[n] = in[n] * m.start_volume;
        }
      else if (n < m.end)
        {
          double d = double (n - m.start) / double (m.end - m.start);
          out[n] = in[n] * ((1 - d) * m.start_volume + d * m.end_volume);
        }
      else // (n >= end)
        {
          out[n] = in[n] * m.end_volume;
        }
    }
}

void
optimize_transient_model (TransientModel& m, vector<double>& signal, const vector<double>& desired_signal)
{
  int nomod = 0;

  vector<double> trsignal;
  while (nomod < 3000)
    {
      transient_scale (m, signal, trsignal);
      double m_delta = vector_delta (trsignal, desired_signal);

      TransientModel new_m = m;
      new_m.start += g_random_int_range (-5, 5);
      new_m.end += g_random_int_range (-5, 5);
      new_m.start_volume += g_random_double_range (-0.001, 0.001);
      new_m.end_volume += g_random_double_range (-0.001, 0.001);

      if (new_m.end > signal.size())
        new_m.end = signal.size();
      if (new_m.start + 10 > new_m.end)
        new_m.start = new_m.end - 10;
      if (new_m.start < 0)
        new_m.start = 0;
      transient_scale (new_m, signal, trsignal);

      double new_m_delta = vector_delta (trsignal, desired_signal);

      if (new_m_delta < m_delta)
        {
          m = new_m;
          nomod = 0;
        }
      else
        {
          nomod++;
        }
    }
}

static void
reconstruct (AudioBlock&     audio_block,
             vector<double>& signal,
             double          mix_freq)
{
  for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
    {
      double smag = audio_block.phases[2 * partial];
      double cmag = audio_block.phases[2 * partial + 1];
      double f    = audio_block.freqs[partial];
      double phase = 0;

      // do a phase optimal reconstruction of that partial
      for (size_t n = 0; n < signal.size(); n++)
        {
          signal[n] += sin (phase) * smag;
          signal[n] += cos (phase) * cmag;
          phase += f / mix_freq * 2.0 * M_PI;
        }
    }
}

vector<double>
spectrum (AudioBlock& block)
{
}

float
mag (float re, float im)
{
  return sqrt (re * re + im * im);
}

struct Attack
{
  double attack_start_ms;
  double attack_end_ms;
};

double
attack_error (const SpectMorph::Audio& audio, const vector< vector<double> >& unscaled_signal, const Attack& attack)
{
  const size_t frames = unscaled_signal.size();
  double total_error = 0;

  for (size_t f = 0; f < frames; f++)
    {
      const vector<double>& frame_signal = unscaled_signal[f];
      size_t zero_values = 0;

      for (size_t n = 0; n < frame_signal.size(); n++)
        {
          const double n_ms = f * audio.frame_step_ms + n * 1000.0 / audio.mix_freq;
          const double scale = (zero_values > 0) ? frame_signal.size() / double (frame_signal.size() - zero_values) : 1.0;
          double env;
          if (n_ms < attack.attack_start_ms)
            {
              env = 0;
              zero_values++;
            }
          else if (n_ms < attack.attack_end_ms)  // during attack
            {
              const double attack_len_ms = attack.attack_end_ms - attack.attack_start_ms;

              env = (n_ms - attack.attack_start_ms) / attack_len_ms;
            }
          else // after attack
            {
              env = 1.0;
            }
          const double value = frame_signal[n] * scale * env;
          const double error = value - audio.contents[f].debug_samples[n];
          total_error += error * error;
        }
    }
  return total_error;
}

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);

  SpectMorph::Audio audio;
  BseErrorType error = SpectMorph::AudioFile::load (argv[1], audio);
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
      vector<double> sines (frame_size);

      reconstruct (audio.contents[i], sines, audio.mix_freq);

      /* compute block size from frame size (smallest 2^k value >= frame_size) */
      uint64 block_size = 1;
      while (block_size < frame_size)
        block_size *= 2;

      // construct window
      vector<float> window (block_size);
      for (guint i = 0; i < window.size(); i++)
        {
          if (i < frame_size)
            window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
          else
            window[i] = 0;
        }

      // apply window to reconstructed signal
      sines.resize (block_size);
      for (guint i = 0; i < sines.size(); i++)
        sines[i] *= window[i];

      // zeropad
      const int    zeropad  = 4;
      sines.resize (block_size * zeropad);
      vector<double> out (block_size * zeropad);

      gsl_power2_fftar (block_size * zeropad, &sines[0], &out[0]);

      vector<double> sines_spectrum;
      for (size_t n = 0; n < audio.contents[i].original_fft.size(); n += 2)
        {
          double re = audio.contents[i].original_fft[n];
          double im = audio.contents[i].original_fft[n + 1];
          spectrum.push_back (sqrt (re * re + im * im));
          sines_spectrum.push_back (mag (out[n], out[n+1]));
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
          printf ("%f %f %f\n", n * 0.5 * audio.mix_freq / spectrum.size(), s, sines_spectrum[n]);
        }
    }
  else if (strcmp (argv[2], "frame") == 0)
    {
      int i = atoi (argv[3]);
      size_t frame_size = audio.contents[i].debug_samples.size();
      vector<double> sines (frame_size);
      reconstruct (audio.contents[i], sines, audio.mix_freq);
      for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
        {
          double v = audio.contents[i].debug_samples[n];
          printf ("%zd %f %f %f\n", n, v, sines[n], v - sines[n]);
        }
#if 0     
      // optimize transient parameter set
      vector<double> trsines;
      TransientModel m;
      m.start_volume = 1;
      m.end_volume = 1;
      m.start = 0;
      m.end = sines.size();
      vector<double> debug_samples (audio.contents[i].debug_samples.begin(), audio.contents[i].debug_samples.end());
      optimize_transient_model (m, sines, debug_samples);
      transient_scale (m, sines, trsines);
      for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
        {
          double v = audio.contents[i].debug_samples[n];
          printf ("%zd %f %f %f\n", n, v, trsines[n], v - trsines[n]);
        }
#endif
    }
  else if (strcmp (argv[2], "frameparams") == 0)
    {
      int i = atoi (argv[3]);
      for(;;)
        {
          double maxm = 0;
          size_t maxp = 0;
          for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
            {
              double m = mag (audio.contents[i].phases[partial * 2], audio.contents[i].phases[partial * 2 + 1]);
              if (m > maxm)
                {
                  maxm = m;
                  maxp = partial;
                }
            }
          if (maxm > 0)
            {
              printf ("%f Hz: %f\n", audio.contents[i].freqs[maxp], bse_db_from_factor (maxm, -200));
              audio.contents[i].phases[maxp * 2] = 0;
              audio.contents[i].phases[maxp * 2 + 1] = 0;
            }
          else
            {
              break;
            }
        }
    }
  else if (strcmp (argv[2], "attack") == 0)
    {
      const size_t frame_size = audio.contents[0].debug_samples.size();
      const size_t frames = 20;

      if (audio.attack_start_ms > 0.01 || audio.attack_end_ms > 0.01)
        {
          printf ("attack values already present: soa=%f, eoa=%f\n", audio.attack_start_ms, audio.attack_end_ms);
          exit (1);
        }
      vector< vector<double> > unscaled_signal;
      for (size_t f = 0; f < frames; f++)
        {
          const AudioBlock& audio_block = audio.contents[f];
          vector<double> frame_signal (frame_size);

          for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
            {
              double smag = audio_block.phases[2 * partial];
              double cmag = audio_block.phases[2 * partial + 1];
              double f    = audio_block.freqs[partial];
              double phase = 0;

              // do a phase optimal reconstruction of that partial
              for (size_t n = 0; n < frame_signal.size(); n++)
                {
                  frame_signal[n] += sin (phase) * smag;
                  frame_signal[n] += cos (phase) * cmag;
                  phase += f / audio.mix_freq * 2.0 * M_PI;
                }
            }
          unscaled_signal.push_back (frame_signal);
        }

      Attack attack;
      int no_modification = 0;
      double error = 1e7;

      attack.attack_start_ms = 0;
      attack.attack_end_ms = 10;
      while (no_modification < 3000)
        {
          double R;
          Attack new_attack = attack;
          if (no_modification < 500)
            R = 100;
          else if (no_modification < 1000)
            R = 20;
          else if (no_modification < 1500)
            R = 1;
          else if (no_modification < 2000)
            R = 0.2;
          else if (no_modification < 2500)
            R = 0.01;

          new_attack.attack_start_ms += g_random_double_range (-R, R);
          new_attack.attack_end_ms += g_random_double_range (-R, R);

          if (new_attack.attack_start_ms < new_attack.attack_end_ms &&
              new_attack.attack_start_ms >= 0 &&
              new_attack.attack_end_ms < 200)
            {
              const double new_error = attack_error (audio, unscaled_signal, new_attack);
#if 0
              printf ("attack=<%f, %f> error=%.17g new_attack=<%f, %f> new_arror=%.17g\n", attack.attack_start_ms, attack.attack_end_ms, error,
                                                                                           new_attack.attack_start_ms, new_attack.attack_end_ms, new_error);
#endif
              if (new_error < error)
                {
                  error = new_error;
                  attack = new_attack;

                  no_modification = 0;
                }
              else
                {
                  no_modification++;
                }
            }
        }
      printf ("## soa=%f, eoa=%f, min_total_error = %f\n", attack.attack_start_ms, attack.attack_end_ms, error);
      audio.attack_start_ms = attack.attack_start_ms;
      audio.attack_end_ms = attack.attack_end_ms;

      SpectMorph::AudioFile::save (argv[1], audio);
#if 0
      size_t GRID_I = 297;
      size_t GRID_J = 297;
      double min_total_error = 1e7;
      for (int i = 0; i < GRID_I; i++)
        {
          for (int j = 0; j < GRID_J; j++)
            {
              Attack attack;

              attack.attack_start_ms = (100.0 * i) / GRID_I; // 100 ms attack len max
              attack.attack_end_ms = attack.attack_start_ms + (100.0 * j) / GRID_J; // 100 ms attack len max

              double total_error = attack_error (audio, unscaled_signal, attack);
              printf ("%f %f %f\n", attack.attack_start_ms, attack.attack_end_ms, total_error);
              min_total_error = min (total_error, min_total_error);
            }
        }
      printf ("## min_total_error = %f\n", min_total_error);
#endif
    }
}
