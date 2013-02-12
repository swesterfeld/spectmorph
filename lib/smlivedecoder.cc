// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlivedecoder.hh"
#include "smmath.hh"
#include "smleakdebugger.hh"

#include <bse/bsemathsignal.hh>
#include <bse/bseblockutils.hh>

#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using Birnet::AlignedArray;
using std::min;

static LeakDebugger leak_debugger ("SpectMorph::LiveDecoder");

#define ANTIALIAS_FILTER_TABLE_SIZE 256

#define DEBUG (0)

static vector<float> antialias_filter_table;

static void
init_aa_filter()
{
  if (antialias_filter_table.empty())
    {
      antialias_filter_table.resize (ANTIALIAS_FILTER_TABLE_SIZE);

      const double db_at_nyquist = -60;

      for (size_t i = 0; i < antialias_filter_table.size(); i++)
        antialias_filter_table[i] = bse_db_to_factor (double (i) / ANTIALIAS_FILTER_TABLE_SIZE * db_at_nyquist);
    }
}

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
  source (NULL),
  sines_enabled (true),
  noise_enabled (true),
  debug_fft_perf_enabled (false),
  original_samples_enabled (false),
  noise_seed (-1),
  sse_samples (NULL)
{
  init_aa_filter();
  leak_debugger.add (this);
  latency_ms = 0;
}

LiveDecoder::LiveDecoder (LiveDecoderSource *source) :
  smset (NULL),
  audio (NULL),
  ifft_synth (NULL),
  noise_decoder (NULL),
  source (source),
  sines_enabled (true),
  noise_enabled (true),
  debug_fft_perf_enabled (false),
  original_samples_enabled (false),
  noise_seed (-1),
  sse_samples (NULL)
{
  init_aa_filter();
  leak_debugger.add (this);
  latency_ms = 0;
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
  leak_debugger.del (this);
}

void
LiveDecoder::set_latency_ms (float new_latency_ms)
{
  latency_ms = new_latency_ms;
}

void
LiveDecoder::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  Audio *best_audio = 0;
  double best_diff = 1e10;

  if (source)
    {
      source->retrigger (channel, freq, midi_velocity, mix_freq);
      best_audio = source->audio();
    }
  else
    {
      if (smset)
        {
          float note = freq_to_note (freq);

          // find best audio candidate
          for (vector<WavSetWave>::iterator wi = smset->waves.begin(); wi != smset->waves.end(); wi++)
            {
              Audio *audio = wi->audio;
              if (audio && wi->channel == channel &&
                           wi->velocity_range_min <= midi_velocity &&
                           wi->velocity_range_max >= midi_velocity)
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
    }
  audio = best_audio;

  if (best_audio)
    {
      frame_size = audio->frame_size_ms * mix_freq / 1000;
      frame_step = audio->frame_step_ms * mix_freq / 1000;
      zero_values_at_start_scaled = audio->zero_values_at_start * mix_freq / audio->mix_freq;
      loop_start_scaled = audio->loop_start * mix_freq / audio->mix_freq;
      loop_end_scaled = audio->loop_end * mix_freq / audio->mix_freq;
      loop_point = (audio->loop_type == Audio::LOOP_NONE) ? -1 : audio->loop_start;

      block_size = NoiseDecoder::preferred_block_size (mix_freq);

      if (noise_decoder)
        delete noise_decoder;
      noise_decoder = new NoiseDecoder (audio->mix_freq, mix_freq, block_size);

      if (noise_seed != -1)
        noise_decoder->set_seed (noise_seed);

      if (ifft_synth)
        delete ifft_synth;
      ifft_synth = new IFFTSynth (block_size, mix_freq, IFFTSynth::WIN_HANNING);

      if (sse_samples)
        delete sse_samples;
      sse_samples = new AlignedArray<float, 16> (block_size);

      pp_inter = PolyPhaseInter::the(); // do not delete

      have_samples = 0;
      pos = 0;
      frame_idx = 0;
      env_pos = 0;
      original_sample_pos = 0;

      pstate[0].clear();
      pstate[1].clear();
      last_pstate = &pstate[0];

      int signed_latency_zero_samples = (latency_ms - best_audio->start_ms) / 1000 * mix_freq;
      if (signed_latency_zero_samples >= 0)
        latency_zero_samples = signed_latency_zero_samples;
      else
        {
          g_warning ("SpectMorph::LiveDecoder: latency problem: latency_zero_samples = %d --> 0\n",
                     signed_latency_zero_samples);
          latency_zero_samples = 0;
        }
    }
  current_freq = freq;
  current_mix_freq = mix_freq;
}

size_t
LiveDecoder::compute_loop_frame_index (size_t frame_idx, Audio *audio)
{
  if (int (frame_idx) > audio->loop_start)
    {
      g_return_val_if_fail (audio->loop_end >= audio->loop_start, frame_idx);

      if (audio->loop_type == Audio::LOOP_FRAME_FORWARD)
        {
          size_t loop_len = audio->loop_end + 1 - audio->loop_start;
          frame_idx = audio->loop_start + (frame_idx - audio->loop_start) % loop_len;
        }
      else if (audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
        {
          size_t loop_len = audio->loop_end - audio->loop_start;
          if (loop_len > 0)
            {
              size_t ping_pong_len = loop_len * 2;
              size_t ping_pong_pos = (frame_idx - audio->loop_start) % ping_pong_len;

              if (ping_pong_pos < loop_len) // ping part of the ping-pong loop (forward)
                {
                  frame_idx = audio->loop_start + ping_pong_pos;
                }
              else                          // pong part of the ping-pong loop (backward)
                {
                  frame_idx = audio->loop_end - (ping_pong_pos - loop_len);
                }
            }
          else
            {
              frame_idx = audio->loop_start;
            }
        }
    }
  return frame_idx;
}

void
LiveDecoder::process (size_t n_values, const float *freq_in, const float *freq_mod_in, float *audio_out)
{
  if (!audio)   // nothing loaded
    {
      std::fill (audio_out, audio_out + n_values, 0);
      return;
    }

  /*
   * delay audio signal to be able to play all samples with normalized latency
   * (start markers should be played at the same time)
   */
  if (latency_zero_samples > 0)
    {
      size_t zero_samples = min (n_values, latency_zero_samples);
      std::fill (audio_out, audio_out + zero_samples, 0);

      // adapt parameters to be able to compute the rest of the block (if any)
      latency_zero_samples -= zero_samples;
      n_values -= zero_samples;

      if (freq_in)
        freq_in += zero_samples;

      if (freq_mod_in)
        freq_mod_in += zero_samples;

      audio_out += zero_samples;
    }

  if (original_samples_enabled)
    {
      for (unsigned int i = 0; i < n_values; i++)
        {
          double want_freq = freq_in ? BSE_SIGNAL_TO_FREQ (freq_in[i]) : current_freq;
          double phase_inc = (want_freq / audio->fundamental_freq) *
                             (audio->mix_freq / current_mix_freq);

          int ipos = original_sample_pos;
          float frac = original_sample_pos - ipos;

          if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
            {
              while (ipos >= (audio->loop_end - audio->zero_values_at_start))
                ipos -= (audio->loop_end - audio->loop_start);
            }
          audio_out[i] = pp_inter->get_sample (audio->original_samples, ipos + frac);

          original_sample_pos += phase_inc;
        }
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

          if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
            {
              int xenv_pos = env_pos;

              if (env_pos > loop_start_scaled)
                {
                  xenv_pos = (env_pos - loop_start_scaled) % (loop_end_scaled - loop_start_scaled);
                  xenv_pos += loop_start_scaled;
                }
              frame_idx = xenv_pos / frame_step;
            }
          else if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
            {
              frame_idx = compute_loop_frame_index (env_pos / frame_step, audio);
            }
          else
            {
              frame_idx = env_pos / frame_step;
              if (loop_point != -1 && frame_idx > size_t (loop_point)) /* if in loop mode: loop current frame */
                frame_idx = loop_point;
            }

          AudioBlock *audio_block_ptr = NULL;
          if (source)
            {
              audio_block_ptr = source->audio_block (frame_idx);
            }
          else if (frame_idx < audio->contents.size())
            {
              audio_block_ptr = &audio->contents[frame_idx];
            }
          if (audio_block_ptr)
            {
              const AudioBlock& audio_block = *audio_block_ptr;

              ifft_synth->clear_partials();

              // point n_pstate to pstate[0] and pstate[1] alternately (one holds points to last state and the other points to new state)
              bool lps_zero = (last_pstate == &pstate[0]);
              vector<PartialState>& new_pstate = lps_zero ? pstate[1] : pstate[0];
              const vector<PartialState>& old_pstate = lps_zero ? pstate[0] : pstate[1];

              new_pstate.clear();  // clear old partial state

              if (sines_enabled)
                {
                  const double phase_factor = block_size * M_PI / current_mix_freq;
                  const double freq_factor = want_freq / audio->fundamental_freq;
                  const double filter_fact = 18000.0 / 44100.0;  // for 44.1 kHz, filter at 18 kHz (higher mix freq => higher filter)
                  const double filter_min_freq = filter_fact * current_mix_freq;

                  size_t old_partial = 0;
                  for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
                    {
                      const double freq = audio_block.freqs[partial] * freq_factor;

                      // anti alias filter:
                      double mag         = audio_block.mags[partial];
                      double phase       = 0; //atan2 (smag, cmag); FIXME: Does initial phase matter? I think not.
                      if (freq > filter_min_freq)
                        {
                          double norm_freq = freq / current_mix_freq;
                          if (norm_freq > 0.5)
                            {
                              // above nyquist freq -> since partials are sorted, there is nothing more to do for this frame
                              break;
                            }
                          else
                            {
                              // between filter_fact and 0.5 (db linear filter)
                              int index = sm_round_positive (ANTIALIAS_FILTER_TABLE_SIZE * (norm_freq - filter_fact) / (0.5 - filter_fact));
                              if (index >= 0)
                                {
                                  if (index < ANTIALIAS_FILTER_TABLE_SIZE)
                                    mag *= antialias_filter_table[index];
                                  else
                                    mag = 0;
                                }
                              else
                                {
                                  // filter magnitude is supposed to be 1.0
                                }
                            }
                        }

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

                              phase = fmod (lphase + lfreq * phase_factor, 2 * M_PI);

                              if (DEBUG)
                                printf ("%zd:L %.17g %.17g %.17g\n", env_pos, lfreq, freq, mag);
                            }
                        }
                      if (DEBUG)
                        printf ("%zd:F %.17g %.17g\n", env_pos, freq, mag);

                      ifft_synth->render_partial (freq, mag, phase);

                      PartialState ps;
                      ps.freq = freq;
                      ps.phase = phase;
                      new_pstate.push_back (ps);
                    }
                }
              last_pstate = &new_pstate;

              if (noise_enabled)
                noise_decoder->process (audio_block, ifft_synth->fft_buffer(), NoiseDecoder::FFT_SPECTRUM);

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

void
LiveDecoder::enable_original_samples (bool eos)
{
  original_samples_enabled = eos;
}

void
LiveDecoder::precompute_tables (float mix_freq)
{
  /* computing one sample (from the source) will ensure that tables (like
   * anti-alias filter table and IFFTSynth window table and FFTW plan) will be
   * available once RT synthesis is needed
   */
  float out;

  retrigger (0, 440, 127, mix_freq);
  process (1, NULL, NULL, &out);
}

void
LiveDecoder::set_noise_seed (int seed)
{
  assert (seed >= -1);
  noise_seed = seed;
}
