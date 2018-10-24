// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminsteditsynth.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::InstEditSynth");

InstEditSynth::InstEditSynth (float mix_freq) :
  mix_freq (mix_freq)
{
  leak_debugger.add (this);
}

InstEditSynth::~InstEditSynth()
{
  leak_debugger.del (this);
}

void
InstEditSynth::load_smset (const string& smset, bool enable_original_samples)
{
  wav_set.load (smset);
  decoder.reset (new LiveDecoder (&wav_set));
  decoder->enable_original_samples (enable_original_samples);
}

static double
note_to_freq (int note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
InstEditSynth::handle_midi_event (const unsigned char *midi_data)
{
  const unsigned char status = (midi_data[0] & 0xf0);
  /* note on: */
  if (status == 0x90 && midi_data[2] != 0) /* note on with velocity 0 => note off */
    {
      if (decoder)
        decoder->retrigger (0, note_to_freq (midi_data[1]), 127, 48000);
      decoder_factor = 1;
    }

  /* note off */
  if (status == 0x80 || (status == 0x90 && midi_data[2] == 0))
    {
      decoder_factor = 0;
    }
}

void
InstEditSynth::process (float *output, size_t n_values)
{
  if (decoder)
    decoder->process (n_values, nullptr, &output[0]);
  else
    zero_float_block (n_values, output);

  for (size_t i = 0; i < n_values; i++)
    {
      output[i] *= decoder_factor;
    }
}
