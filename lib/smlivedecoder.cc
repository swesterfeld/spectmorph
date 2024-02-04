// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smlivedecoder.hh"
#include "smlivedecoderfilter.hh"
#include "smmath.hh"
#include "smleakdebugger.hh"
#include "smutils.hh"
#include "smrtmemory.hh"

#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::min;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::LiveDecoder");

#define ANTIALIAS_FILTER_TABLE_SIZE 256

#define DEBUG (0)

static vector<float> antialias_filter_table;
static std::mutex aa_mutex;

static void
init_aa_filter()
{
  std::lock_guard lg (aa_mutex);

  if (antialias_filter_table.empty())
    {
      antialias_filter_table.resize (ANTIALIAS_FILTER_TABLE_SIZE);

      const double db_at_nyquist = -60;

      for (size_t i = 0; i < antialias_filter_table.size(); i++)
        antialias_filter_table[i] = db_to_factor (double (i) / ANTIALIAS_FILTER_TABLE_SIZE * db_at_nyquist);
    }
}

static inline double
fmatch (double f1, double f2)
{
  return f2 < (f1 * 1.05) && f2 > (f1 * 0.95);
}

static inline double
truncate_phase (double phase)
{
  // truncate phase to interval [0:2*pi]; like fmod (phase, 2 * M_PI) but faster
  phase *= 1 / (2 * M_PI);
  phase -= int (phase);
  phase *= 2 * M_PI;

  return phase;
}

LiveDecoder::LiveDecoder (float mix_freq) :
  smset (NULL),
  audio (NULL),
  block_size (NoiseDecoder::preferred_block_size (mix_freq)),
  ifft_synth (block_size, mix_freq, IFFTSynth::WIN_HANNING),
  noise_decoder (mix_freq, block_size),
  source (NULL),
  sines_enabled (true),
  noise_enabled (true),
  debug_fft_perf_enabled (false),
  original_samples_enabled (false),
  loop_enabled (true),
  start_skip_enabled (false),
  mix_freq (mix_freq),
  noise_seed (-1),
  sse_samples (block_size),
  vibrato_enabled (false)
{
  leak_debugger.add (this);

  init_aa_filter();
  set_unison_voices (1, 0);
  /* avoid malloc during synthesis */
  pstate[0].reserve (PARTIAL_STATE_RESERVE);
  pstate[1].reserve (PARTIAL_STATE_RESERVE);
  unison_phases[0].reserve (PARTIAL_STATE_RESERVE * MAX_UNISON_VOICES);
  unison_phases[1].reserve (PARTIAL_STATE_RESERVE * MAX_UNISON_VOICES);
  unison_freq_factor.reserve (MAX_UNISON_VOICES);

  pp_inter = PolyPhaseInter::the(); // do not delete
}

LiveDecoder::LiveDecoder (WavSet *smset, float mix_freq) :
  LiveDecoder (mix_freq)
{
  this->smset = smset;
}

LiveDecoder::LiveDecoder (LiveDecoderSource *source, float mix_freq) :
  LiveDecoder (mix_freq)
{
  this->source = source;
}

LiveDecoder::~LiveDecoder()
{
  leak_debugger.del (this);
}

void
LiveDecoder::retrigger (int channel, float freq, int midi_velocity)
{
  Audio *best_audio = 0;
  double best_diff = 1e10;

  if (source)
    {
      source->retrigger (channel, freq, midi_velocity);
      best_audio = source->audio();
    }
  else
    {
      if (smset)
        {
          float note = sm_freq_to_note (freq);

          // find best audio candidate
          for (vector<WavSetWave>::iterator wi = smset->waves.begin(); wi != smset->waves.end(); wi++)
            {
              Audio *audio = wi->audio;
              if (audio && wi->channel == channel &&
                           wi->velocity_range_min <= midi_velocity &&
                           wi->velocity_range_max >= midi_velocity)
                {
                  float audio_note = sm_freq_to_note (audio->fundamental_freq);

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
      frame_step = audio->frame_step_ms * mix_freq / 1000;
      zero_values_at_start_scaled = audio->zero_values_at_start * mix_freq / audio->mix_freq;
      loop_start_scaled = audio->loop_start * mix_freq / audio->mix_freq;
      loop_end_scaled = audio->loop_end * mix_freq / audio->mix_freq;
      loop_point = (get_loop_type() == Audio::LOOP_NONE) ? -1 : audio->loop_start;

      /* start skip: skip the first half block to avoid fade-in at start
       * this will produce clicks unless an external envelope is applied
       */
      if (start_skip_enabled)
        zero_values_at_start_scaled += block_size / 2;

      zero_float_block (block_size, &sse_samples[0]);

      if (noise_seed != -1)
        noise_decoder.set_seed (noise_seed);

      have_samples = 0;
      pos = 0;
      frame_idx = 0;
      env_pos = 0;
      original_sample_pos = 0;
      original_samples_norm_factor = db_to_factor (audio->original_samples_norm_db);
      old_portamento_stretch = 1;

      done_state = DoneState::ACTIVE;

      // reset partial state vectors
      pstate[0].clear();
      pstate[1].clear();
      last_pstate = &pstate[0];

      // reset unison phases
      unison_phases[0].clear();
      unison_phases[1].clear();

      // setup vibrato state
      vibrato_phase = 0;
      vibrato_env = 0;
    }
  filter_latency_compensation = true;
  current_freq = freq;
}

void
LiveDecoder::set_source (LiveDecoderSource *source)
{
  if (source != this->source)
    {
      this->source = source;
      audio = nullptr; /* stop playback */
    }
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
LiveDecoder::process_internal (size_t n_values, const float *freq_in, float *audio_out)
{
  assert (audio); // need selected (triggered) audio to use this function

  if (original_samples_enabled)
    {
      /* we can skip the resampler if the phase increment is always 1.0
       * this ensures that the original samples are reproduced exactly
       * in this case
       */
      bool need_resample = true;
      const double phase_inc = (current_freq / audio->fundamental_freq) *
                               (audio->mix_freq / mix_freq);
      if (fabs (phase_inc - 1.0) < 1e-6)
        need_resample = false;

      for (unsigned int i = 0; i < n_values; i++)
        {
          double want_freq = current_freq;
          double phase_inc = (want_freq / audio->fundamental_freq) *
                             (audio->mix_freq / mix_freq);

          int ipos = original_sample_pos;
          float frac = original_sample_pos - ipos;

          if (get_loop_type() == Audio::LOOP_TIME_FORWARD)
            {
              while (ipos >= (audio->loop_end - audio->zero_values_at_start))
                ipos -= (audio->loop_end - audio->loop_start);
            }

          if (need_resample)
            {
              audio_out[i] = pp_inter->get_sample (audio->original_samples, ipos + frac) * original_samples_norm_factor;
            }
          else
            {
              if (ipos >= 0 && size_t (ipos) < audio->original_samples.size())
                audio_out[i] = audio->original_samples[ipos] * original_samples_norm_factor;
              else
                audio_out[i] = 0;
            }

          original_sample_pos += phase_inc;
        }
      if (original_sample_pos > audio->original_samples.size() && get_loop_type() != Audio::LOOP_TIME_FORWARD)
        {
          if (done_state == DoneState::ACTIVE)
            done_state = DoneState::ALMOST_DONE;
        }
      return;
    }

  const double portamento_env_step = 1 / old_portamento_stretch;
  unsigned int i = 0;
  while (i < n_values)
    {
      if (pos >= have_samples)
        {
          double want_freq = current_freq;

#if 0
          std::copy (&sse_samples[block_size / 2], &sse_samples[block_size], &sse_samples[0]);
          zero_float_block (block_size / 2, &sse_samples[block_size / 2]);
#endif

          if (get_loop_type() == Audio::LOOP_TIME_FORWARD)
            {
              size_t xenv_pos = env_pos;

              if (xenv_pos > loop_start_scaled)
                {
                  xenv_pos = (xenv_pos - loop_start_scaled) % (loop_end_scaled - loop_start_scaled);
                  xenv_pos += loop_start_scaled;
                }
              frame_idx = xenv_pos / frame_step;
            }
          else if (get_loop_type() == Audio::LOOP_FRAME_FORWARD || get_loop_type() == Audio::LOOP_FRAME_PING_PONG)
            {
              frame_idx = compute_loop_frame_index (env_pos / frame_step, audio);
            }
          else
            {
              frame_idx = env_pos / frame_step;
              if (loop_point != -1 && frame_idx > size_t (loop_point)) /* if in loop mode: loop current frame */
                frame_idx = loop_point;
            }

          RTAudioBlock audio_block (rt_memory_area);
          bool         have_audio_block = false;
          if (source)
            {
              have_audio_block = source->rt_audio_block (frame_idx, audio_block);
            }
          else if (frame_idx < audio->contents.size())
            {
              audio_block.assign (audio->contents[frame_idx]);
              have_audio_block = true;
            }
          if (have_audio_block)
            {
              float portamento_stretch = freq_in[i] / current_freq;
              assert (audio_block.freqs.size() == audio_block.mags.size());

              ifft_synth.clear_partials();

              // point n_pstate to pstate[0] and pstate[1] alternately (one holds points to last state and the other points to new state)
              bool lps_zero = (last_pstate == &pstate[0]);
              vector<PartialState>& new_pstate = lps_zero ? pstate[1] : pstate[0];
              const vector<PartialState>& old_pstate = lps_zero ? pstate[0] : pstate[1];
              vector<float>& unison_new_phases = lps_zero ? unison_phases[1] : unison_phases[0];
              const vector<float>& unison_old_phases = lps_zero ? unison_phases[0] : unison_phases[1];

              if (unison_voices != 1)
                {
                  // check unison phases size corresponds to old partial state size
                  assert (unison_voices * old_pstate.size() == unison_old_phases.size());
                }
              new_pstate.clear();         // clear old partial state
              unison_new_phases.clear();  // and old unison phase information

              if (sines_enabled)
                {
                  const double phase_factor = block_size * M_PI / mix_freq;
                  const double filter_fact = 18000.0 / 44100.0;  // for 44.1 kHz, filter at 18 kHz (higher mix freq => higher filter)
                  const double filter_min_freq = filter_fact * mix_freq;

                  size_t old_partial = 0;
                  for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
                    {
                      const double freq = audio_block.freqs_f (partial) * want_freq;

                      // anti alias filter:
                      double mag         = audio_block.mags_f (partial);
                      double phase       = 0; //atan2 (smag, cmag); FIXME: Does initial phase matter? I think not.

                      // portamento: FIXME: is using freq_in[i] directly here the best way?
                      const double portamento_freq = audio_block.freqs_f (partial) * freq_in[i];
                      if (portamento_freq > filter_min_freq)
                        {
                          double norm_freq = portamento_freq / mix_freq;
                          if (norm_freq > 0.5)
                            {
                              // above nyquist freq -> since partials are sorted, there is nothing more to do for this frame
                              // FIXME: this may not work as expected with new style portamento, because we change the
                              // rate of the resynthesis dynamically
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
                      bool freq_match = false;
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
                          freq_match = fmatch (lfreq, freq);
                        }
                      if (DEBUG)
                        printf ("%d:F %.17g %.17g\n", int (env_pos), freq, mag);

                      if (unison_voices == 1)
                        {
                          if (freq_match)
                            {
                              // matching freq -> compute new phase
                              const double lfreq = old_pstate[old_partial].freq;
                              const double lphase = old_pstate[old_partial].phase;

                              phase = truncate_phase (lphase + ifft_synth.quantized_freq (lfreq * old_portamento_stretch) * phase_factor);

                              if (DEBUG)
                                printf ("%d:L %.17g %.17g %.17g\n", int (env_pos), lfreq, freq, mag);
                            }
                          //ifft_synth.render_partial (freq * portamento_stretch, mag, phase);
                        }
                      else
                        {
                          mag *= unison_gain;

                          for (int i = 0; i < unison_voices; i++)
                            {
                              if (freq_match)
                                {
                                  const double lfreq = old_pstate[old_partial].freq;
                                  const double lphase = unison_old_phases[old_partial * unison_voices + i];

                                  phase = truncate_phase (lphase + lfreq * phase_factor * unison_freq_factor[i] * portamento_stretch);
                                }
                              else
                                {
                                  // randomize start phase for unison

                                  phase = unison_phase_random_gen.random_double_range (0, 2 * M_PI);
                                }

                              //ifft_synth.render_partial (freq * unison_freq_factor[i] * portamento_stretch, mag, phase);

                              unison_new_phases.push_back (phase);
                            }
                        }

                      PartialState ps;
                      ps.freq = freq;
                      ps.mag = mag;
                      ps.phase = phase;
                      new_pstate.push_back (ps);
                    }
                  zero_float_block (block_size, &sse_samples[0]);
                  for (auto ps : old_pstate)
                    {
                      // phase at center of the block
                      auto phase = ps.phase + ifft_synth.quantized_freq (ps.freq * old_portamento_stretch) * phase_factor;
                      // phase at start of the block
                      phase -= ifft_synth.quantized_freq (ps.freq * portamento_stretch) * phase_factor;
                      while (phase < 0)
                        phase += 2 * M_PI;
                      phase = truncate_phase (phase);

                      ifft_synth.render_partial (ps.freq * portamento_stretch, ps.mag, phase);
                    }
                  ifft_synth.get_samples (&sse_samples[0], IFFTSynth::REPLACE);
                  std::copy (&sse_samples[block_size / 2], &sse_samples[block_size], &sse_samples[0]);
                  std::fill (&sse_samples[block_size / 2], &sse_samples[block_size], 0.f);
#if 0
#endif

#if 0
                  for (auto ps : old_pstate)
                    for (uint i = 0; i < block_size / 2; i++)
                      {
                        auto phase = truncate_phase (ps.phase + ps.freq * phase_factor * old_portamento_stretch);
                        sse_samples[i] += sin (i * ps.freq * portamento_stretch / mix_freq * 2 * M_PI + phase) * ps.mag * window_cos ((block_size / 2.0 - i) / (block_size / 2.0) - 1);
                      }
#endif
                  ifft_synth.clear_partials();
                  for (auto ps : new_pstate)
                    {
                      ifft_synth.render_partial (ps.freq * portamento_stretch, ps.mag, ps.phase);
                    }
                  ifft_synth.get_samples (&sse_samples[0], IFFTSynth::ADD);
#if 0
                    for (int i = 0; i < block_size / 2; i++)
                      {
                        sse_samples[i] += sin (i * ps.freq * portamento_stretch / mix_freq * 2 * M_PI + ps.phase) * ps.mag * window_cos (i / (block_size / 2.0) - 1);
                      }
#endif
                }
              last_pstate = &new_pstate;

#if 0
              if (noise_enabled)
                noise_decoder.process (audio_block, ifft_synth.fft_buffer(), NoiseDecoder::FFT_SPECTRUM, portamento_stretch);

              if (noise_enabled || sines_enabled || debug_fft_perf_enabled)
                {
                  float *samples = &sse_samples[0];
                  ifft_synth.get_samples (samples, IFFTSynth::ADD);
                }
#endif
              if (pos != 0) // pos == 0 => initial refill
                {
                  pos -= block_size / 2;
                  /* adjust remaining fractional position matching to new stretch */
                  pos *= old_portamento_stretch / portamento_stretch;
                }
              old_portamento_stretch = portamento_stretch;
            }
          else
            {
              /* FIXME: need to generate null sample block here (setup pos, clear out samples) */
              if (done_state == DoneState::ACTIVE)
                done_state = DoneState::ALMOST_DONE;
            }
          have_samples = block_size / 2;
          rt_memory_area->free_all();
        }

      g_assert (have_samples > 0);
      if (env_pos >= zero_values_at_start_scaled)
        {
          // decode envelope
          const double time_ms = env_pos * 1000.0 / mix_freq;
          if (time_ms < audio->attack_start_ms)
            {
              audio_out[i++] = 0;
              pos++;
              env_pos += portamento_env_step;
            }
          else if (time_ms < audio->attack_end_ms)
            {
              audio_out[i++] = sse_samples[pos] * (time_ms - audio->attack_start_ms) / (audio->attack_end_ms - audio->attack_start_ms);
              pos++;
              env_pos += portamento_env_step;
            }
          else // envelope is 1 -> copy data efficiently
            {
              int ipos = pos;
              float frac = pos - ipos;
              audio_out[i] = sse_samples[ipos] * (1 - frac) + sse_samples[ipos + 1] * frac;
              pos += freq_in[i] / (current_freq * old_portamento_stretch);
              i++;
              env_pos += portamento_env_step;
#if 0
              size_t can_copy = min (have_samples, n_values - i);

              memcpy (audio_out + i, &sse_samples[pos], sizeof (float) * can_copy);
              i += can_copy;
              pos += can_copy;
              env_pos += can_copy * portamento_env_step;
              have_samples -= can_copy;
#endif
            }
        }
      else
        {
          // skip sample
          pos++;
          env_pos += portamento_env_step;
        }
    }
}

void
LiveDecoder::process_portamento (size_t n_values, const float *freq_in, float *audio_out)
{
  assert (audio); // need selected (triggered) audio to use this function

  float fake_freq_in[n_values];
  if (!freq_in)
    {
      std::fill (fake_freq_in, fake_freq_in + n_values, current_freq);
      freq_in = fake_freq_in;
    }
  process_internal (n_values, freq_in, audio_out);
}

void
LiveDecoder::process_vibrato (size_t n_values, const float *freq_in, float *audio_out)
{
  float vib_freq_in[n_values];

  /* how many samples has the attack phase? */
  const float attack_samples  = vibrato_attack / 1000.0 * mix_freq;

  /* compute per sample envelope increment */
  const float vibrato_env_inc   = attack_samples > 1.0 ? 1.0 / attack_samples : 1.0;

  const float vibrato_phase_inc = vibrato_frequency / mix_freq * 2 * M_PI;
  const float vibrato_depth_factor = pow (2, vibrato_depth / 1200.0) - 1;

  for (size_t i = 0; i < n_values; i++)
    {
      vib_freq_in[i] = freq_in ? freq_in[i] : current_freq;

      if (vibrato_env > 1.0) // attack phase done?
        {
          vib_freq_in[i] *= 1 + sin (vibrato_phase) * vibrato_depth_factor;
        }
      else
        {
          vibrato_env += vibrato_env_inc;

          vib_freq_in[i] *= 1 + sin (vibrato_phase) * vibrato_depth_factor * vibrato_env;
        }

      vibrato_phase += vibrato_phase_inc;
    }
  vibrato_phase = fmod (vibrato_phase, 2 * M_PI);

  process_portamento (n_values, vib_freq_in, audio_out);
}

void
LiveDecoder::process_with_filter (size_t n_values, const float *freq_in, float *audio_out, bool ramp)
{
  if (vibrato_enabled)
    {
      process_vibrato (n_values, freq_in, audio_out);
    }
  else
    {
      process_portamento (n_values, freq_in, audio_out);
    }

  if (filter)
    {
      if (ramp)
        {
          // the audio input for the filter can start at a non-zero value:
          // use a 1ms ramp to reduce the "click" that is processed by the filter
          uint ramp_len = mix_freq * 0.001f;

          float audio_ramp[ramp_len];
          float amp = 0;
          float delta_amp = audio_out[0] / (ramp_len + 1);
          for (uint i = 0; i < ramp_len; i++)
            {
              amp += delta_amp;
              audio_ramp[i] = amp;
            }

          filter->process (ramp_len, audio_ramp);
        }
      filter->process (n_values, audio_out);
    }
}

void
LiveDecoder::process (RTMemoryArea& rt_memory_area, size_t n_values, const float *freq_in, float *audio_out)
{
  if (source)
    audio = source->audio();  // sources can stop providing audio data while playing

  if (!audio)   // nothing loaded
    {
      std::fill (audio_out, audio_out + n_values, 0);
      done_state = DoneState::DONE;
      return;
    }
  /* required during processing */
  assert (!this->rt_memory_area);
  this->rt_memory_area = &rt_memory_area;

  /* ensure that time_offset_ms() is only called during live decoder process */
  assert (!in_process);
  in_process = true;
  start_env_pos = env_pos;
  /*
   * split processing into small blocks
   *  -> limit n_values to keep portamento stretch settings up-to-date
   *  -> provide accurate modulation data to filter
   */
  const size_t max_n_values = MAX_N_VALUES;
  const size_t orig_n_values = n_values;
  const float *orig_audio_out = audio_out;

  if (n_values && filter && filter_latency_compensation)
    {
      // latency compensation for filter oversampling: throw away a few samples at the start

      int idelay = filter->idelay();
      assert (idelay > 0);

      float junk_audio_out[idelay];
      float junk_freq_in[idelay];
      if (freq_in)
        {
          for (int i = 0; i < idelay; i++)
            junk_freq_in[i] = freq_in[0];

          process_with_filter (idelay, junk_freq_in, junk_audio_out, true);
        }
      else
        {
          process_with_filter (idelay, nullptr, junk_audio_out, true);
        }

      filter_latency_compensation = false;
    }

  while (n_values > 0)
    {
      size_t todo_values = min (n_values, max_n_values);

      process_with_filter (todo_values, freq_in, audio_out, false);

      if (freq_in)
        freq_in += todo_values;

      audio_out += todo_values;
      n_values -= todo_values;
    }

  /* update done state ALMOST_DONE => DONE: we need
   *  - done state ALMOST_DONE
   *  - a silent output buffer
   */
  if (done_state == DoneState::ALMOST_DONE)
    {
      size_t i = 0;
      while (i < orig_n_values)
        {
          if (orig_audio_out[i] != 0.0)
            break;

          i++;
        }
      if (i == orig_n_values)
        done_state = DoneState::DONE;
    }
  this->rt_memory_area = nullptr;
  in_process = false;
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
LiveDecoder::enable_loop (bool eloop)
{
  loop_enabled = eloop;
}

void
LiveDecoder::enable_start_skip (bool ess)
{
  start_skip_enabled = ess;
}

void
LiveDecoder::precompute_tables (float mix_freq)
{
  size_t block_size = NoiseDecoder::preferred_block_size (mix_freq);

  NoiseDecoder noise_decoder (mix_freq, block_size);
  IFFTSynth ifft_synth (block_size, mix_freq, IFFTSynth::WIN_HANNING);

  noise_decoder.precompute_tables();
  ifft_synth.precompute_tables();

  init_aa_filter();
}

void
LiveDecoder::set_noise_seed (int seed)
{
  assert (seed >= -1);
  noise_seed = seed;
}

Audio::LoopType
LiveDecoder::get_loop_type()
{
  assert (audio);

  if (loop_enabled)
    {
      return audio->loop_type;
    }
  else
    {
      return Audio::LOOP_NONE;
    }
}

void
LiveDecoder::set_unison_voices (int voices, float detune)
{
  assert (voices > 0);

  unison_voices = voices;

  if (voices == 1)
    return;

  /* setup unison frequency factors for unison voices */
  unison_freq_factor.resize (voices);

  for (size_t i = 0; i < unison_freq_factor.size(); i++)
    {
      const float detune_cent = -detune/2 + i / float (voices - 1) * detune;
      unison_freq_factor[i] = pow (2, detune_cent / 1200);
    }

  /* take into account the more unison voices we add up, the louder the result
   * will be, so compensate for this:
   *
   *   -> each time the number of voices is doubled, the signal level is increased by
   *      a factor of sqrt (2)
   */
  unison_gain = 1 / sqrt (voices);

  /* resize unison phase array to match pstate */
  const bool lps_zero = (last_pstate == &pstate[0]);
  const vector<PartialState>& old_pstate = lps_zero ? pstate[0] : pstate[1];
  vector<float>& unison_old_phases = lps_zero ? unison_phases[0] : unison_phases[1];

  if (unison_old_phases.size() != old_pstate.size() * unison_voices)
    {
      unison_old_phases.resize (old_pstate.size() * unison_voices);

      for (float& phase : unison_old_phases)
        {
          /* since the position of the partials changed, randomization is really
           * the best we can do here */
          phase = unison_phase_random_gen.random_double_range (0, 2 * M_PI);
        }
    }
}

void
LiveDecoder::set_vibrato (bool enabled, float depth, float frequency, float attack)
{
  vibrato_enabled     = enabled;
  vibrato_depth       = depth;
  vibrato_frequency   = frequency;
  vibrato_attack      = attack;
}

double
LiveDecoder::current_pos() const
{
  if (!audio)
    return -1;

  if (original_samples_enabled)
    return original_sample_pos * 1000.0 / audio->mix_freq;

  return frame_idx * audio->frame_step_ms - audio->zero_values_at_start * 1000.0 / audio->mix_freq;
}

double
LiveDecoder::fundamental_note() const
{
  if (!audio)
    return -1;

  return sm_freq_to_note (audio->fundamental_freq);
}

bool
LiveDecoder::done() const
{
  return done_state == DoneState::DONE;
}

double
LiveDecoder::time_offset_ms() const
{
  /* LiveDecoder::process() produces samples by IFFTSynth - possibly more than once per block
   *
   * This function provides the time offset of the IFFTSynth call relative to the start of
   * the sample block generated by process(). LFOs (and other operators) can use this
   * information for jitter-free timing.
   */
  assert (in_process);
  return 1000 * (env_pos - start_env_pos) / mix_freq;
}

void
LiveDecoder::set_filter (LiveDecoderFilter *new_filter)
{
  filter = new_filter;
}
