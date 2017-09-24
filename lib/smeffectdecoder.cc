// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
  } state;

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
  m_audio.attack_start_ms      = 0;
  m_audio.attack_end_ms        = 0;
  m_audio.zeropad              = 4;
  m_audio.loop_type            = Audio::LOOP_NONE;
  m_audio.zero_values_at_start = 0;
  m_audio.sample_count         = 2 << 31;
}

EffectDecoder::EffectDecoder (LiveDecoderSource *source) :
  original_source (source),
  skip_source (new EffectDecoderSource (source))
{
  chain_decoder.reset (new LiveDecoder (original_source));
  use_skip_source = false;
}

EffectDecoder::~EffectDecoder()
{
}

void
EffectDecoder::set_config (MorphOutput *output, float mix_freq)
{
  if (output->adsr())
    {
      if (!use_skip_source) // enable skip source
        {
          chain_decoder.reset (new LiveDecoder (skip_source));
          chain_decoder->enable_start_skip (true);
          use_skip_source = true;
        }
      skip_source->set_skip (output->adsr_skip());

      if (!adsr_envelope)
        adsr_envelope.reset (new ADSREnvelope());

      adsr_envelope->set_config (output->adsr_attack(),
                                 output->adsr_decay(),
                                 output->adsr_sustain(),
                                 output->adsr_release(),
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

  chain_decoder->enable_noise (output->noise());
  chain_decoder->enable_sines (output->sines());

  if (output->unison()) // unison?
    chain_decoder->set_unison_voices (output->unison_voices(), output->unison_detune());
  else
    chain_decoder->set_unison_voices (1, 0);

  chain_decoder->set_vibrato (output->vibrato(), output->vibrato_depth(), output->vibrato_frequency(), output->vibrato_attack());
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
}

void
EffectDecoder::process (size_t       n_values,
                        const float *freq_in,
                        float       *audio_out)
{
  g_assert (chain_decoder);

  chain_decoder->process (n_values, freq_in, audio_out);

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
}

bool
EffectDecoder::done()
{
  if (adsr_envelope)
    return adsr_envelope->done();
  else
    return simple_envelope->done();
}
