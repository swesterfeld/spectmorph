// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputmodule.hh"

using namespace SpectMorph;

EffectDecoder::EffectDecoder (LiveDecoderSource *source)
{
  chain = new LiveDecoder (source);
  chain2 = new LiveDecoder (source);
  chain3 = new LiveDecoder (source);

  chorus_active = false; // shouldn't matter, as we expect enable_chorus() to be called anyway
}

EffectDecoder::~EffectDecoder()
{
  delete chain;
  delete chain2;
  delete chain3;
}

void
EffectDecoder::enable_noise (bool en)
{
  chain->enable_noise (en);
  chain2->enable_noise (en);
  chain3->enable_noise (en);
}

void
EffectDecoder::enable_sines (bool es)
{
  chain->enable_sines (es);
  chain2->enable_sines (es);
  chain3->enable_sines (es);
}

void
EffectDecoder::enable_chorus (bool ec)
{
  chorus_active = ec;
}

void
EffectDecoder::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  float fact2 = pow (2,(6/1200.));
  float fact3 = pow (2,(-7/1200.));

  chain->retrigger (channel, freq, midi_velocity, mix_freq);
  chain2->retrigger (channel, freq * fact2, midi_velocity, mix_freq);
  chain3->retrigger (channel, freq * fact3, midi_velocity, mix_freq);
}

void
EffectDecoder::process (size_t       n_values,
                        const float *freq_in,
                        const float *freq_mod_in,
                        float       *audio_out)
{
  if (!chorus_active)
    {
      chain->process (n_values, freq_in, freq_mod_in, audio_out);
      return;
    }

  float output[n_values];
  float output2[n_values];
  float output3[n_values];

  chain->process (n_values, freq_in, freq_mod_in, output);
  chain2->process (n_values, freq_in, freq_mod_in, output2);
  chain3->process (n_values, freq_in, freq_mod_in, output3);

  for (size_t i = 0; i < n_values; i++)
    audio_out[i] = output[i] + output2[i] + output3[i];
}
