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
  m_audio.zeropad              = 4;
  m_audio.loop_type            = Audio::LOOP_NONE;
}

EffectDecoder::EffectDecoder (MorphOutputModule *output_module, LiveDecoderSource *source) :
  output_module (output_module),
  original_source (source),
  skip_source (new EffectDecoderSource (source))
{
  filter_callback = [this]() { filter_cutoff = this->output_module->filter_cutoff_mod(); };

  chain_decoder.reset (new LiveDecoder (original_source));
  chain_decoder->set_filter_callback (filter_callback);
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
          chain_decoder->set_filter_callback (filter_callback);
          chain_decoder->enable_start_skip (true);
          use_skip_source = true;
        }
      skip_source->set_skip (cfg->adsr_skip);

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
          chain_decoder->set_filter_callback (filter_callback);
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

  switch (cfg->filter_type)
    {
      case MorphOutput::FILTER_LP1:
        filter.set_mode (LadderVCFMode::LP1);
        break;
      case MorphOutput::FILTER_LP2:
        filter.set_mode (LadderVCFMode::LP2);
        break;
      case MorphOutput::FILTER_LP3:
        filter.set_mode (LadderVCFMode::LP3);
        break;
      case MorphOutput::FILTER_LP4:
        filter.set_mode (LadderVCFMode::LP4);
        break;
    }

  filter_enabled = cfg->filter;
  filter_resonance = cfg->filter_resonance;
}

static float
freq_to_note (float freq)
{
  return 69 + 12 * log (freq / 440) / log (2);
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

  filter.reset();
  filter_envelope.start (mix_freq);

  float note = freq_to_note (freq);
  float keytrack = 100; /* FIXME: FILTER: should be configurable */
  float delta_cent = (note - 60) * keytrack;
  filter_keytrack_factor = exp2f (delta_cent * (1 / 1200.f));
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

  if (filter_enabled)
    {
      float junk[n_values];
      const float *inputs[2] = { audio_out, junk };
      float *outputs[2] = { audio_out, junk };

      float freq[n_values];
      for (uint i = 0; i < n_values; i++)
        freq[i] = filter_cutoff * filter_keytrack_factor * exp2f (filter_envelope.get_next() * filter_depth_octaves);

      filter.run_block (n_values, filter_cutoff, filter_resonance * 0.01f, inputs, outputs, true, false, freq, nullptr, nullptr, nullptr);
    }
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
