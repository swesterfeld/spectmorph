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
#include "smaudio.hh"
#include "smmain.hh"
#include <assert.h>
#include <bse/bsemathsignal.h>
#include <bse/gslfft.h>

using namespace SpectMorph;
using std::vector;
using std::min;
using std::string;

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
attack_error (const SpectMorph::Audio& audio, const vector< vector<double> >& unscaled_signal, const Attack& attack, vector<double>& out_scale)
{
  const size_t frames = unscaled_signal.size();
  double total_error = 0;

  for (size_t f = 0; f < frames; f++)
    {
      const vector<double>& frame_signal = unscaled_signal[f];
      size_t zero_values = 0;
      double scale;

      for (size_t n = 0; n < frame_signal.size(); n++)
        {
          const double n_ms = f * audio.frame_step_ms + n * 1000.0 / audio.mix_freq;
          double env;
          scale = (zero_values > 0) ? frame_signal.size() / double (frame_signal.size() - zero_values) : 1.0;
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
      out_scale[f] = scale;
    }
  return total_error;
}

void
check_usage (int argc, int need_argc, const string& usage)
{
  if (argc != need_argc)
    {
      printf ("usage: smextract <sm_file> %s\n", usage.c_str());
      exit (1);
    }
}

Audio
load_or_die (const string& filename, const string& mode)
{
  Audio audio;
  AudioLoadOptions load_options = AUDIO_LOAD_DEBUG;

  if (mode == "fundamental-freq" || mode == "freq"
  ||  mode == "frameparams" || mode == "noiseparams" || mode == "attack" ||
      mode == "zero-values-at-start" || mode == "mix-freq")
    load_options = AUDIO_SKIP_DEBUG;

  BseErrorType error = audio.load (filename, load_options);
  if (error)
    {
      fprintf (stderr, "can't load file: %s\n",filename.c_str());
      exit (1);
    }
  return audio;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc < 3)
    {
      printf ("usage: smextract <sm_file> <mode> [ <mode_specific_args> ]\n");
      exit (1);
    }

  const string& mode = argv[2];

  Audio audio = load_or_die (argv[1], mode);
  int frame_size = audio.frame_size_ms * audio.mix_freq / 1000;
  bool need_save = false;

  if (mode == "freq")
    {
      check_usage (argc, 5, "freq <freq_min> <freq_max>");

      float freq_min = atof (argv[3]);
      float freq_max = atof (argv[4]);

      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          const AudioBlock& block = audio.contents[i];
          for (size_t n = 0; n < block.freqs.size(); n++)
            {
              if (block.freqs[n] > freq_min && block.freqs[n] < freq_max)
                {
                  printf ("%zd %f %f\n", i, block.freqs[n], mag (block.phases[n * 2], block.phases[n * 2 + 1]));
                }
            }
        }
    }
  else if (mode == "spectrum")
    {
      check_usage (argc, 4, "spectrum <frame_no>");

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
  else if (mode == "frame")
    {
      check_usage (argc, 4, "frame <frame_no>");

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
  else if (mode == "frameparams")
    {
      check_usage (argc, 4, "frameparams <frame_no>");

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
  else if (mode == "noiseparams")
    {
      check_usage (argc, 4, "noiseparams <frame_no>");

      int f = atoi (argv[3]);
      for (int i = 0; i < audio.contents[f].noise.size(); i++)
        {
          printf ("%f\n", audio.contents[f].noise[i]);
        }
    }
  else if (mode == "attack")
    {
      check_usage (argc, 3, "attack");

      printf ("start of attack: %.2f ms\n", audio.attack_start_ms);
      printf ("  end of attack: %.2f ms\n", audio.attack_end_ms);
    }
  else if (mode == "size")
    {
      check_usage (argc, 3, "size");

      size_t phase_bytes = 0, freq_bytes = 0, debug_samples_bytes = 0, original_fft_bytes = 0, noise_bytes = 0;
      for (size_t f = 0; f < audio.contents.size(); f++)
        {
          phase_bytes += audio.contents[f].phases.size() * sizeof (float);
          freq_bytes += audio.contents[f].freqs.size() * sizeof (float);
          debug_samples_bytes += audio.contents[f].debug_samples.size() * sizeof (float);
          original_fft_bytes += audio.contents[f].original_fft.size() * sizeof (float);
          noise_bytes += audio.contents[f].noise.size() * sizeof (float);
        }
      printf ("phases       : %d bytes\n", phase_bytes);
      printf ("frequencies  : %d bytes\n", freq_bytes);
      printf ("dbgsamples   : %d bytes\n", debug_samples_bytes);
      printf ("orig_fft     : %d bytes\n", original_fft_bytes);
      printf ("noise        : %d bytes\n", noise_bytes);
    }
  else if (mode == "fundamental-freq")
    {
      check_usage (argc, 3, "fundamental-freq");

      printf ("fundamental-freq: %f\n", audio.fundamental_freq);
    }
  else if (mode == "mix-freq")
    {
      check_usage (argc, 3, "mix-freq");

      printf ("mix-freq: %f\n", audio.mix_freq);
    }
  else if (mode == "auto-tune")
    {
      check_usage (argc, 3, "auto-tune");

      const double freq_min = audio.fundamental_freq * 0.8;
      const double freq_max = audio.fundamental_freq / 0.8;
      double freq_sum = 0, mag_sum = 0;

      for (size_t f = 0; f < audio.contents.size(); f++)
        {
          double position_percent = f * 100.0 / audio.contents.size();
          if (position_percent >= 40 && position_percent <= 60)
            {
              const AudioBlock& block = audio.contents[f];
              double best_freq = -1;
              double best_mag = 0;
              for (size_t n = 0; n < block.freqs.size(); n++)
                {
                  if (block.freqs[n] > freq_min && block.freqs[n] < freq_max)
                    {
                      double m = mag (block.phases[n * 2], block.phases[n * 2 + 1]);
                      if (m > best_mag)
                        {
                          best_mag = m;
                          best_freq = block.freqs[n];
                        }
                    }
                }
              if (best_mag > 0)
                {
                  freq_sum += best_freq * best_mag;
                  mag_sum += best_mag;
                }
            }
        }
      if (mag_sum > 0)
        {
          double fundamental_freq = freq_sum / mag_sum;
          audio.fundamental_freq = fundamental_freq;
          printf ("%.17g %.17g\n", audio.fundamental_freq, fundamental_freq);

          need_save = true;
        }
    }
  else if (mode == "loopparams")
    {
      check_usage (argc, 3, "loopparams");

      printf ("frames: %d\n", audio.contents.size());
      printf ("loop point: %d\n", audio.loop_point);
    }
  else if (mode == "auto-loop")
    {
      check_usage (argc, 4, "auto-loop <percent>");

      double percent = atof (argv[3]);
      if (percent < 0 || percent > 100)
        {
          fprintf (stderr, "bad loop percentage: %f\n", percent);
          exit (1);
        }

      audio.loop_point = audio.contents.size() * percent / 100;
      if (audio.loop_point < 0)
        audio.loop_point = 0;
      if (audio.loop_point >= (audio.contents.size() - 1))
        audio.loop_point = audio.contents.size() - 1;

      need_save = true;
    }
  else if (mode == "tail-loop")
    {
      check_usage (argc, 3, "tail-loop");

      int loop_point = -1;
      const int frame_step = audio.frame_step_ms * audio.mix_freq / 1000;

      // we need the largest frame that doesn't include any data beyond the original file end
      for (size_t i = 0; i < audio.contents.size(); i++)
        {
          if (i * frame_step + frame_size < audio.sample_count)
            loop_point = i;
        }
      audio.loop_point = loop_point;
      need_save = true;
    }
  else if (mode == "zero-values-at-start")
    {
      check_usage (argc, 3, "zero-values-at-start");

      printf ("zero-values-at-start: %d\n", audio.zero_values_at_start);
    }
  if (need_save)
    {
      if (audio.save (argv[1]) != BSE_ERROR_NONE)
        {
          fprintf (stderr, "error saving file: %s\n", argv[1]);
          exit (1);
        }
    }
}
