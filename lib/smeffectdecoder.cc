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
  LiveDecoderSource *m_source = nullptr;
  Audio              m_audio;
  float              m_skip;
public:
  EffectDecoderSource();

  void retrigger (int channel, float freq, int midi_velocity) override;
  Audio* audio() override;
  bool rt_audio_block (size_t index, RTAudioBlock& out_block) override;

  void set_skip (float m_skip);
  void set_source (LiveDecoderSource *source);
};

}

void
EffectDecoderSource::retrigger (int channel, float freq, int midi_velocity)
{
  if (m_source)
    m_source->retrigger (channel, freq, midi_velocity);
}

Audio*
EffectDecoderSource::audio()
{
  return &m_audio;
}

bool
EffectDecoderSource::rt_audio_block (size_t index, RTAudioBlock& out_block)
{
  const double time_ms = index + m_skip; // 1ms frame step

  return MorphUtils::get_normalized_block (m_source, time_ms, out_block);
}

void
EffectDecoderSource::set_skip (float skip)
{
  m_skip = skip;
}

void
EffectDecoderSource::set_source (LiveDecoderSource *source)
{
  m_source = source;
}

EffectDecoderSource::EffectDecoderSource() :
  m_skip (0)
{
  m_audio.fundamental_freq     = 440;
  m_audio.mix_freq             = 48000;
  m_audio.frame_size_ms        = 1;
  m_audio.frame_step_ms        = 1;
  m_audio.zeropad              = 4;
  m_audio.loop_type            = Audio::LOOP_NONE;
}

EffectDecoder::EffectDecoder (MorphOutputModule *output_module, float mix_freq) :
  output_module (output_module),
  chain_decoder (mix_freq)
{
  skip_source.reset (new EffectDecoderSource());
  adsr_envelope.reset (new ADSREnvelope());
  simple_envelope.reset (new SimpleEnvelope (mix_freq));
}

EffectDecoder::~EffectDecoder()
{
}

void
EffectDecoder::set_config (const MorphOutput::Config *cfg, LiveDecoderSource *source, float mix_freq)
{
  adsr_enabled = cfg->adsr;

  if (cfg->adsr)
    {
      chain_decoder.set_source (skip_source.get());
      chain_decoder.enable_start_skip (true);

      skip_source->set_source (source);
      skip_source->set_skip (cfg->adsr_skip);

      adsr_envelope->set_config (cfg->adsr_attack,
                                 cfg->adsr_decay,
                                 cfg->adsr_sustain,
                                 cfg->adsr_release,
                                 mix_freq);
    }
  else
    {
      chain_decoder.set_source (source);
      chain_decoder.enable_start_skip (false);
    }

  chain_decoder.enable_noise (cfg->noise);
  chain_decoder.enable_sines (cfg->sines);

  if (cfg->unison) // unison?
    chain_decoder.set_unison_voices (cfg->unison_voices, cfg->unison_detune);
  else
    chain_decoder.set_unison_voices (1, 0);

  chain_decoder.set_vibrato (cfg->vibrato, cfg->vibrato_depth, cfg->vibrato_frequency, cfg->vibrato_attack);

  if (cfg->filter)
    {
      live_decoder_filter.set_config (output_module, cfg, mix_freq);
      if (!filter_enabled)
        live_decoder_filter.retrigger (sm_freq_to_note (current_freq));

      chain_decoder.set_filter (&live_decoder_filter);
    }
  else
    chain_decoder.set_filter (nullptr);

  filter_enabled = cfg->filter;
}

void
EffectDecoder::retrigger (int channel, float freq, int midi_velocity)
{
  if (filter_enabled)
    live_decoder_filter.retrigger (sm_freq_to_note (freq));

  current_freq = freq;

  if (adsr_enabled)
    adsr_envelope->retrigger();
  else
    simple_envelope->retrigger();

  chain_decoder.retrigger (channel, freq, midi_velocity);
}

void
EffectDecoder::process (RTMemoryArea& rt_memory_area,
                        size_t        n_values,
                        const float  *freq_in,
                        float        *audio_out)
{
  chain_decoder.process (rt_memory_area, n_values, freq_in, audio_out);

  if (adsr_enabled)
    adsr_envelope->process (n_values, audio_out);
  else
    simple_envelope->process (n_values, audio_out);
}

void
EffectDecoder::release()
{
  if (adsr_enabled)
    adsr_envelope->release();
  else
    simple_envelope->release();

  /* FIXME: is it a good idea to do this here? */
  live_decoder_filter.release();
}

bool
EffectDecoder::done()
{
  if (adsr_enabled)
    return adsr_envelope->done();
  else
    return simple_envelope->done();
}

double
EffectDecoder::time_offset_ms() const
{
  return chain_decoder.time_offset_ms();
}
