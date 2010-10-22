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
#include <bse/bseblockutils.hh>

#include <stdio.h>

using SpectMorph::LiveDecoder;
using std::vector;
using Birnet::AlignedArray;
using std::min;

#define ANTIALIAS_FILTER_TABLE_SIZE 256

static vector<float> antialias_filter_table;

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
  debug_fft_perf_enabled (false),
  sse_samples (NULL)
{
  if (antialias_filter_table.empty())
    {
      antialias_filter_table.resize (ANTIALIAS_FILTER_TABLE_SIZE);

      const double db_at_nyquist = -60;

      for (size_t i = 0; i < antialias_filter_table.size(); i++)
        antialias_filter_table[i] = bse_db_to_factor (double (i) / ANTIALIAS_FILTER_TABLE_SIZE * db_at_nyquist);
    }
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
  if (sse_samples)
    {
      delete sse_samples;
      sse_samples = NULL;
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

      block_size = NoiseDecoder::preferred_block_size (mix_freq);

      if (noise_decoder)
        delete noise_decoder;
      noise_decoder = new NoiseDecoder (audio->mix_freq, mix_freq, block_size);


      if (ifft_synth)
        delete ifft_synth;
      ifft_synth = new IFFTSynth (block_size, mix_freq, IFFTSynth::WIN_HANNING);

      if (sse_samples)
        delete sse_samples;
      sse_samples = new AlignedArray<float, 16> (block_size);

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

          std::copy (&(*sse_samples)[block_size / 2], &(*sse_samples)[block_size], &(*sse_samples)[0]);
          zero_float_block (block_size / 2, &(*sse_samples)[block_size / 2]);

          frame_idx = env_pos / frame_step;
          if (loop_point != -1 && frame_idx > loop_point) /* if in loop mode: loop current frame */
            frame_idx = loop_point;

          if ((frame_idx + 1) < audio->contents.size()) // FIXME: block selection pass
            {
              const AudioBlock& audio_block = audio->contents[frame_idx];

              ifft_synth->clear_partials();

              // point n_pstate to pstate[0] and pstate[1] alternately (one holds points to last state and the other points to new state)
              bool lps_zero = (last_pstate == &pstate[0]);
              vector<PartialState>& new_pstate = lps_zero ? pstate[1] : pstate[0];
              const vector<PartialState>& old_pstate = lps_zero ? pstate[0] : pstate[1];

              new_pstate.clear();  // clear old partial state

              size_t old_partial = 0;
              for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
                {
                  const double freq = audio_block.freqs[partial] * want_freq / audio->fundamental_freq;

                  // anti alias filter:
                  double filter_fact = 18000.0 / 44100.0;  // for 44.1 kHz, filter at 18 kHz (higher mix freq => higher filter)
                  double norm_freq   = freq / current_mix_freq;
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
                          int index = 0.5 + ANTIALIAS_FILTER_TABLE_SIZE * (norm_freq - filter_fact) / (0.5 - filter_fact);
                          if (index >= 0)
                            {
                              if (index < ANTIALIAS_FILTER_TABLE_SIZE)
                                aa_filter_mag = antialias_filter_table[index];
                              else
                                aa_filter_mag = 0;
                            }
                          else
                            {
                              // filter magnitude is supposed to be 1.0
                            }
                        }
                    }
                  double mag = audio_block.mags[partial] * aa_filter_mag;
                  double phase = 0; //atan2 (smag, cmag); FIXME: Does initial phase matter? I think not.

                  /*
                   * increment old_partial as long as there is a better candidate (closer to freq)
                   */
                  if (!old_pstate.empty())
                    {
                      double best_fdiff = fabs (old_pstate[old_partial].freq - freq);

                      while ((old_partial + 1) < old_pstate.size())
                        {
                          double fdiff = fabs (old_pstate[old_partial + 1].freq - freq);
                          if (fdiff < best_fdiff)
                            {
                              old_partial++;
                              best_fdiff = fdiff;
                            }
                          else
                            {
                              break;
                            }
                        }
                      const double lfreq = old_pstate[old_partial].freq;
                      if (fmatch (lfreq, freq))
                        {
                          // matching freq -> compute new phase
                          const double lphase = old_pstate[old_partial].phase;
                          const double phase_delta = 2 * M_PI * lfreq / current_mix_freq;

                          // FIXME: I have no idea why we have to /subtract/ the phase
                          // here, and not /add/, but this way it works

                          phase = lphase - block_size / 2 * phase_delta;
                        }
                    }

                  if (sines_enabled)
                    {
                      ifft_synth->render_partial (freq, mag, -phase);

                      PartialState ps;
                      ps.freq = freq;
                      ps.phase = phase;
                      new_pstate.push_back (ps);
                    }
                }

              last_pstate = &new_pstate;

              if (noise_enabled)
                noise_decoder->process (audio->contents[frame_idx], ifft_synth->fft_buffer(), NoiseDecoder::FFT_SPECTRUM);

              if (noise_enabled || sines_enabled || debug_fft_perf_enabled)
                {
                  float *samples = &(*sse_samples)[0];
                  ifft_synth->get_samples (samples, IFFTSynth::ADD);
                }
            }
          pos = 0;
          have_samples = block_size / 2;
        }

      g_assert (have_samples > 0);
      if (env_pos >= zero_values_at_start_scaled)
        {
          // decode envelope
          const double time_ms = env_pos * 1000.0 / current_mix_freq;
          if (time_ms < audio->attack_start_ms)
            {
              audio_out[i++] = 0;
              pos++;
              env_pos++;
              have_samples--;
            }
          else if (time_ms < audio->attack_end_ms)
            {
              audio_out[i++] = (*sse_samples)[pos] * (time_ms - audio->attack_start_ms) / (audio->attack_end_ms - audio->attack_start_ms);
              pos++;
              env_pos++;
              have_samples--;
            }
          else // envelope is 1 -> copy data efficiently
            {
              size_t can_copy = min (have_samples, n_values - i);

              memcpy (audio_out + i, &(*sse_samples)[pos], sizeof (float) * can_copy);
              i += can_copy;
              pos += can_copy;
              env_pos += can_copy;
              have_samples -= can_copy;
            }
        }
      else
        {
          // skip sample
          pos++;
          env_pos++;
          have_samples--;
        }
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

void
LiveDecoder::enable_debug_fft_perf (bool dfp)
{
  debug_fft_perf_enabled = dfp;
}
