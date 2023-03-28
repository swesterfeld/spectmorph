// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphoutputmodule.hh"
#include "smmorphutils.hh"

using namespace SpectMorph;

namespace SpectMorph
{

class SimpleEnvelope
{
  double decrement;
  double level;
  enum class State {
    ON,
    RELEASE,
    DONE
  } state = State::DONE;

public:
  SimpleEnvelope (float mix_freq)
  {
    const float release_ms = 150; /* FIXME: this should be set by the user */
    decrement = (1000.0 / mix_freq) / release_ms;
  }
  void
  retrigger()
  {
    state = State::ON;
    level = 1.0;
  }
  void
  release()
  {
    state = State::RELEASE;
  }
  bool
  done() const
  {
    return state == State::DONE;
  }
  void
  process (size_t n_values, float *values)
  {
    if (state == State::ON)
      {
        return; // nothing
      }
    else if (state == State::RELEASE)
      {
        for (size_t i = 0; i < n_values; i++)
          {
            level -= decrement;
            if (level > 0)
              values[i] *= level;
            else
              values[i] = 0;
          }
        if (level < 0)
          state = State::DONE;
      }
    else // state == State::DONE
      {
        zero_float_block (n_values, values);
      }
  }
};

class EffectDecoderSource : public LiveDecoderSource
{
  LiveDecoderSource *source;
  Audio              m_audio;
  float              m_skip;
public:
  explicit EffectDecoderSource (LiveDecoderSource *source);

  void retrigger (int channel, float freq, int midi_velocity, float mix_freq) override;
  Audio* audio() override;
  AudioBlock* audio_block (size_t index) override;

  void set_skip (float m_skip);
};

}

void
EffectDecoderSource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  source->retrigger (channel, freq, midi_velocity, mix_freq);
}

Audio*
EffectDecoderSource::audio()
{
  return &m_audio;
}

AudioBlock*
EffectDecoderSource::audio_block (size_t index)
{
  const double time_ms = index + m_skip; // 1ms frame step

  return MorphUtils::get_normalized_block_ptr (source, time_ms);
}

void
EffectDecoderSource::set_skip (float skip)
{
  m_skip = skip;
}

EffectDecoderSource::EffectDecoderSource (LiveDecoderSource *source) :
  source (source),
  m_skip (0)
{
  m_audio.fundamental_freq     = 440;
  m_audio.mix_freq             = 48000;
  m_audio.frame_size_ms        = 1;
  m_audio.frame_step_ms        = 1;
  m_audio.zeropad              = 4;
  m_audio.loop_type            = Audio::LOOP_NONE;
}

EffectDecoder::EffectDecoder (MorphOutputModule *output_module, LiveDecoderSource *source) :
  output_module (output_module),
  original_source (source),
  skip_source (new EffectDecoderSource (source))
{
  chain_decoder.reset (new LiveDecoder (original_source));
  use_skip_source = false;
}

EffectDecoder::~EffectDecoder()
{
}

/* FIXME: FILTER: dedup */
static float
exp_percent (float p, float min_out, float max_out, float slope)
{
  /* exponential curve from 0 to 1 with configurable slope */
  const double x = (pow (2, (p / 100.) * slope) - 1) / (pow (2, slope) - 1);

  /* rescale to interval [min_out, max_out] */
  return x * (max_out - min_out) + min_out;
}

/* FIXME: FILTER: dedup */
static float
xparam_percent (float p, float min_out, float max_out, float slope)
{
  /* rescale xparam function to interval [min_out, max_out] */
  return sm_xparam (p / 100.0, slope) * (max_out - min_out) + min_out;
}

void
EffectDecoder::set_config (const MorphOutput::Config *cfg, float mix_freq)
{
  if (cfg->adsr)
    {
      if (!use_skip_source) // enable skip source
        {
          chain_decoder.reset (new LiveDecoder (skip_source.get()));
          chain_decoder->enable_start_skip (true);
          use_skip_source = true;
        }
      skip_source->set_skip (cfg->adsr_skip);

      simple_envelope.reset();
      if (!adsr_envelope)
        adsr_envelope.reset (new ADSREnvelope());

      adsr_envelope->set_config (cfg->adsr_attack,
                                 cfg->adsr_decay,
                                 cfg->adsr_sustain,
                                 cfg->adsr_release,
                                 mix_freq);
    }
  else
    {
      if (use_skip_source) // use original source (no skip)
        {
          chain_decoder.reset (new LiveDecoder (original_source));
          use_skip_source = false;
        }
      adsr_envelope.reset();

      if (!simple_envelope)
        simple_envelope.reset (new SimpleEnvelope (mix_freq));
    }

  chain_decoder->enable_noise (cfg->noise);
  chain_decoder->enable_sines (cfg->sines);

  if (cfg->unison) // unison?
    chain_decoder->set_unison_voices (cfg->unison_voices, cfg->unison_detune);
  else
    chain_decoder->set_unison_voices (1, 0);

  chain_decoder->set_vibrato (cfg->vibrato, cfg->vibrato_depth, cfg->vibrato_frequency, cfg->vibrato_attack);

  // filter
  float attack  = xparam_percent (cfg->filter_attack, 2, 5000, 3) / 1000;
  float decay   = xparam_percent (cfg->filter_decay, 2, 5000, 3) / 1000;
  float release = exp_percent (cfg->filter_release, 2, 200, 3) / 1000; /* FIXME: FILTER: this may not be the best solution */
  float sustain = cfg->filter_sustain;
  if (0)
    {
      printf ("%.2f ms -  %.2f ms  -  %.2f ms\n",
          attack * 1000,
          decay * 1000,
          release * 1000);
    }

  filter_envelope.set_shape (FilterEnvelope::Shape::LINEAR);
  filter_envelope.set_delay (0);
  filter_envelope.set_attack (attack);
  filter_envelope.set_hold (0);
  filter_envelope.set_decay (decay);
  filter_envelope.set_sustain (sustain);
  filter_envelope.set_release (release);
  filter_depth_octaves =  cfg->filter_depth / 12;

  filter_enabled = cfg->filter;
  if (filter_enabled)
    {
      live_decoder_filter.set_config (output_module, cfg, mix_freq);
      chain_decoder->set_filter (&live_decoder_filter);
    }
  else
    chain_decoder->set_filter (nullptr);
}

void
EffectDecoder::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  g_assert (chain_decoder);

  if (adsr_envelope)
    adsr_envelope->retrigger();
  else
    simple_envelope->retrigger();

  chain_decoder->retrigger (channel, freq, midi_velocity, mix_freq);

  filter_envelope.start (mix_freq);
  filter_first = true;
}

void
EffectDecoder::process_with_filter (size_t       n_values,
                                    const float *freq_in,
                                    float       *audio_out)
{
#if 0
  if (filter_enabled)
    {
      auto filter_process_block = [&] (auto& filter)
        {
          auto gen_filter_input = [&] (float *freq_in, float *reso_in, float *drive_in, uint count)
            {
              for (uint i = 0; i < count; i++)
                {
                  freq_in[i] = filter_cutoff_smooth.get_next() * exp2f (filter_envelope.get_next() * filter_depth_octaves);
                  reso_in[i] = filter_resonance_smooth.get_next();
                  drive_in[i] = filter_drive_smooth.get_next();
                }
            };
          const bool const_freq = filter_cutoff_smooth.is_constant() && filter_envelope.is_constant();
          const bool const_reso = filter_resonance_smooth.is_constant();
          const bool const_drive = filter_drive_smooth.is_constant();

          if (const_freq && const_reso && const_drive)
            {
              /* use more efficient version of the filter computation if all parameters are constants */
              float freq, reso, drive;
              gen_filter_input (&freq, &reso, &drive, 1);

              filter.set_freq (freq);
              filter.set_reso (reso);
              filter.set_drive (drive);
              filter.process_block (n_values, audio_out);
            }
          else
            {
              /* generic version: pass per-sample values for freq, reso and drive */
              float freq_in[n_values], reso_in[n_values], drive_in[n_values];
              gen_filter_input (freq_in, reso_in, drive_in, n_values);

              filter.process_block (n_values, audio_out, nullptr, freq_in, reso_in, drive_in);
            }
        };

    }
#endif
}

void
EffectDecoder::process (size_t       n_values,
                        const float *freq_in,
                        float       *audio_out)
{
  g_assert (chain_decoder);
  chain_decoder->process (n_values, freq_in, audio_out);

#if 0

  if (filter_enabled && filter_first && n_values)
    {
      // latency compensation for filter oversampling: throw away a few samples at the start
      int idelay = 0;

      if (filter_type == MorphOutput::FILTER_TYPE_LADDER)
        idelay = ladder_filter.delay();
      else if (filter_type == MorphOutput::FILTER_TYPE_SALLEN_KEY)
        idelay = sk_filter.delay();

      assert (idelay > 0);

      float junk_audio_out[idelay];
      float junk_freq_in[idelay];

      if (freq_in)
        {
          for (int i = 0; i < idelay; i++)
            junk_freq_in[i] = freq_in[0];

          process_with_filter (idelay, junk_freq_in, junk_audio_out);
        }
      else
        {
          process_with_filter (idelay, nullptr, junk_audio_out);
        }

      filter_first = false;
    }

  process_with_filter (n_values, freq_in, audio_out);
#endif

  if (adsr_envelope)
    adsr_envelope->process (n_values, audio_out);
  else
    simple_envelope->process (n_values, audio_out);
}

void
EffectDecoder::release()
{
  if (adsr_envelope)
    adsr_envelope->release();
  else
    simple_envelope->release();

  if (filter_enabled)
    filter_envelope.stop();
}

bool
EffectDecoder::done()
{
  if (adsr_envelope)
    return adsr_envelope->done();
  else
    return simple_envelope->done();
}

double
EffectDecoder::time_offset_ms() const
{
  return chain_decoder->time_offset_ms();
}
