/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
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

#include "smlivedecoder.hh"
#include "smmath.hh"

#include <bse/bsemathsignal.h>

#include <stdio.h>

using SpectMorph::LiveDecoder;
using std::vector;
using Birnet::AlignedArray;

static float
freq_to_note (float freq)
{
  return 69 + 12 * log (freq / 440) / log (2);
}
static inline double
fmatch (double f1, double f2)
{
  return f2 < (f1 * 1.05) && f2 > (f1 * 0.95);
}

LiveDecoder::LiveDecoder (WavSet *smset) :
  smset (smset),
  audio (NULL),
  ifft_synth (NULL),
  noise_decoder (NULL),
  sines_enabled (true),
  noise_enabled (true),
  decoded_sse_samples (NULL)
{
}

LiveDecoder::~LiveDecoder()
{
  if (ifft_synth)
    {
      delete ifft_synth;
      ifft_synth = NULL;
    }
  if (noise_decoder)
    {
      delete noise_decoder;
      noise_decoder = NULL;
    }
  if (decoded_sse_samples)
    {
      delete decoded_sse_samples;
      decoded_sse_samples = NULL;
    }
}

void
LiveDecoder::retrigger (int channel, float freq, float mix_freq)
{
  double best_diff = 1e10;
  Audio *best_audio = 0;

  if (smset)
    {
      float note = freq_to_note (freq);

      // find best audio candidate
      for (vector<WavSetWave>::iterator wi = smset->waves.begin(); wi != smset->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel)
            {
              float audio_note = freq_to_note (audio->fundamental_freq);

              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }

  audio = best_audio;

  if (best_audio)
    {
      frame_size = audio->frame_size_ms * mix_freq / 1000;
      frame_step = audio->frame_step_ms * mix_freq / 1000;
      zero_values_at_start_scaled = audio->zero_values_at_start * mix_freq / audio->mix_freq;
      loop_point = audio->loop_point;

      if (noise_decoder)
        delete noise_decoder;
      noise_decoder = new NoiseDecoder (audio->mix_freq, mix_freq);

      block_size = noise_decoder->preferred_block_size();

      if (ifft_synth)
        delete ifft_synth;
      ifft_synth = new IFFTSynth (block_size, mix_freq, IFFTSynth::WIN_HANNING);

      if (decoded_sse_samples)
        delete decoded_sse_samples;
      decoded_sse_samples = new AlignedArray<float, 16> (block_size);

      samples.resize (block_size);
      zero_float_block (block_size, &samples[0]);

      have_samples = 0;
      pos = 0;
      frame_idx = 0;
      env_pos = 0;

      pstate[0].clear();
      pstate[1].clear();
      last_pstate = &pstate[0];
    }
  current_freq = freq;
  current_mix_freq = mix_freq;
}

void
LiveDecoder::process (size_t n_values, const float *freq_in, const float *freq_mod_in, float *audio_out)
{
  if (!audio)   // nothing loaded
    {
      std::fill (audio_out, audio_out + n_values, 0);
      return;
    }

  unsigned int i = 0;
  while (i < n_values)
    {
      if (have_samples == 0)
        {
          double want_freq = freq_in ? BSE_SIGNAL_TO_FREQ (freq_in[i]) : current_freq;

          std::copy (samples.begin() + block_size / 2, samples.end(), samples.begin());
          zero_float_block (block_size / 2, &samples[block_size / 2]);

          frame_idx = env_pos / frame_step;
          if (loop_point != -1 && frame_idx > loop_point) /* if in loop mode: loop current frame */
            frame_idx = loop_point;

          if ((frame_idx + 1) < audio->contents.size()) // FIXME: block selection pass
            {
              Frame frame (audio->contents[frame_idx]);

              ifft_synth->clear_partials();

              // point n_pstate to pstate[0] and pstate[1] alternately (one holds points to last state and the other points to new state)
              bool lps_zero = (last_pstate == &pstate[0]);
              vector<PartialState>& new_pstate = lps_zero ? pstate[1] : pstate[0];
              const vector<PartialState>& old_pstate = lps_zero ? pstate[0] : pstate[1];

              new_pstate.clear();  // clear old partial state

              size_t old_partial = 0;
              for (size_t partial = 0; partial < frame.freqs.size(); partial++)
                {
                  frame.freqs[partial] *= want_freq / audio->fundamental_freq;

                  // anti alias filter:
                  double filter_fact = 18000.0 / 44100.0;  // for 44.1 kHz, filter at 18 kHz (higher mix freq => higher filter)
                  double norm_freq   = frame.freqs[partial] / current_mix_freq;
                  double aa_filter_mag = 1.0;
                  if (norm_freq > filter_fact)
                    {
                      if (norm_freq > 0.5)
                        {
                          // above nyquist freq -> since partials are sorted, there is nothing more to do for this frame
                          break;
                        }
                      else
                        {
                          // between filter_fact and 0.5 (db linear filter)
                          const double db_at_nyquist = -60;
                          aa_filter_mag = bse_db_to_factor ((norm_freq - filter_fact) / (0.5 - filter_fact) * db_at_nyquist);
                        }
                    }
                  double smag = frame.phases[partial * 2];
                  double cmag = frame.phases[partial * 2 + 1];
                  double mag = sqrt (smag * smag + cmag * cmag) * aa_filter_mag;
                  double phase = 0; //atan2 (smag, cmag); FIXME: Does initial phase matter? I think not.

                  /*
                   * ensure that frame.freqs[old_partial] is the biggest frequency smaller than frame.freqs[partial]
                   * by incrementing old_partial as long as there is a better candidate
                   */
                  while ((old_partial + 1) < old_pstate.size() && old_pstate[old_partial + 1].freq < frame.freqs[partial])
                    old_partial++;

                  double best_fdiff = 1e12;

                  // check: - biggest frequency smaller than frame.freqs[partial]   (i == 0)
                  //        - smallest frequency bigger than frame.freqs[partial]   (i == 1)
                  for (size_t i = 0; i < 2; i++)
                    {
                      if ((old_partial + i) < old_pstate.size())
                        {
                          const double lfreq = old_pstate[old_partial + i].freq;
                          if (fmatch (lfreq, frame.freqs[partial]))
                            {
                              // find best phase
                              double fdiff = fabs (lfreq - frame.freqs[partial]);
                              if (fdiff < best_fdiff)
                                {
                                  const double lphase = old_pstate[old_partial + i].phase;
                                  const double phase_delta = 2 * M_PI * lfreq / current_mix_freq;

                                  // FIXME: I have no idea why we have to /subtract/ the phase
                                  // here, and not /add/, but this way it works

                                  phase = lphase - block_size / 2 * phase_delta;
                                  best_fdiff = fdiff;
                                }
                            }
                        }
                    }

                  if (sines_enabled)
                    {
                      ifft_synth->render_partial (frame.freqs[partial], mag, -phase);

                      PartialState ps;
                      ps.freq = frame.freqs[partial];
                      ps.phase = phase;
                      new_pstate.push_back (ps);
                    }
                }

              last_pstate = &new_pstate;

              decoded_data.resize (block_size);

              if (sines_enabled)
                {
                  float *decoded_samples = &(*decoded_sse_samples)[0];
                  ifft_synth->get_samples (decoded_samples);
                  for (size_t i = 0; i < block_size; i++)
                    samples[i] += decoded_samples[i];
                }
              if (noise_enabled)
                {
                  noise_decoder->process (frame, decoded_data);
                  for (size_t i = 0; i < block_size; i++)
                    samples[i] += decoded_data[i];
                }
            }
          pos = 0;
          have_samples = block_size / 2;
        }

      g_assert (have_samples > 0);
      if (env_pos >= zero_values_at_start_scaled)
        {
          audio_out[i] = samples[pos];

          // decode envelope
          const double time_ms = env_pos * 1000.0 / current_mix_freq;
          if (time_ms < audio->attack_start_ms)
            {
              audio_out[i] = 0;
            }
          else if (time_ms < audio->attack_end_ms)
            {
              audio_out[i] *= (time_ms - audio->attack_start_ms) / (audio->attack_end_ms - audio->attack_start_ms);
            } // else envelope is 1

          // do not skip sample
          i++;
        }
      else
        {
          // skip sample
        }
      pos++;
      env_pos++;
      have_samples--;
    }
}

void
LiveDecoder::enable_noise (bool en)
{
  noise_enabled = en;
}

void
LiveDecoder::enable_sines (bool es)
{
  sines_enabled = es;
}
