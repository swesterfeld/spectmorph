// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputmodule.hh"

using namespace SpectMorph;

EffectDecoder::EffectDecoder (LiveDecoderSource *source) :
  source (source)
{
}

EffectDecoder::~EffectDecoder()
{
}

void
EffectDecoder::set_config (MorphOutput *output)
{
  size_t unison_voices = output->chorus() ? output->unison_voices() : 1;

  if (unison_voices != chain_decoders.size())
    {
      chain_decoders.resize (unison_voices);

      for (auto& dec : chain_decoders)
        dec.reset (new LiveDecoder (source));
    }

  for (auto& dec : chain_decoders)
    {
      dec->enable_noise (output->noise());
      dec->enable_sines (output->sines());
      dec->enable_phase_randomization (unison_voices > 1);
    }
}

void
EffectDecoder::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  if (chain_decoders.size() == 1)
    {
      chain_decoders[0]->retrigger (channel, freq, midi_velocity, mix_freq);
      return;
    }

  float spread = 6;
  float detune_factor = pow (2,(spread/1200.));
  float freq_l = freq / detune_factor;
  float freq_h = freq * detune_factor;

  for (size_t i = 0; i < chain_decoders.size(); i++)
    {
      const float detune_freq = freq_l + (freq_h - freq_l) / (chain_decoders.size() - 1) * i;

      chain_decoders[i]->retrigger (channel, detune_freq, midi_velocity, mix_freq);
    }
}

void
EffectDecoder::process (size_t       n_values,
                        const float *freq_in,
                        const float *freq_mod_in,
                        float       *audio_out)
{
  if (chain_decoders.size() == 1)
    {
      chain_decoders[0]->process (n_values, freq_in, freq_mod_in, audio_out);
      return;
    }

  zero_float_block (n_values, audio_out);

  for (auto& dec : chain_decoders)
    {
      float output[n_values];

      dec->process (n_values, freq_in, freq_mod_in, output);

      for (size_t i = 0; i < n_values; i++)
        audio_out[i] += output[i];
    }
}
