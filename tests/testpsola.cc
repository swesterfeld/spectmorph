// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smwavset.hh"
#include "smmain.hh"
#include "smlivedecoder.hh"

#include <cassert>

using namespace SpectMorph;

using std::vector;

/*
 * this is a proof of concept implementation of the TD-PSOLA algorithm on SpectMorph data
 *
 * it should provide more realistic pitch shifting for the human voice because
 * it preserves the formants of the original input
 */
int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  assert (argc == 3);

  WavSet wav_set;
  Error error = wav_set.load (argv[1]);
  assert (!error);

  Audio *audio = nullptr;
  for (auto& wave : wav_set.waves)
    {
      if (wave.midi_note == 45) // 110 Hz
        audio = wave.audio;
    }
  assert (audio);
  double freq = 110;
  int analysis_len = 44100 / freq;
  const double SR = 44100;
  auto synth_block = [&](const AudioBlock& block) {
    vector<float> out (analysis_len * 2);
    for (size_t i = 0; i < block.freqs.size(); i++)
      {
        double phase = 0;
        for (size_t pos = 0; pos < out.size(); pos++)
          {
            out[pos] += sin (phase) * block.mags_f (i) * window_cos ((analysis_len - int (pos)) / double (analysis_len));
            phase += block.freqs_f (i) * freq * 2 * M_PI / SR;
          }
      }
    return out;
  };
  int note = atoi (argv[2]);
  double dest_freq = 440 * exp2 ((note - 69) / 12.);
  int x = 0;
  int synthesis_len = analysis_len * freq / dest_freq;
  vector<float> synth_data (5 * SR);
  double block_index = 0;
  while (x < synth_data.size())
    {
      AudioBlock block (audio->contents[LiveDecoder::compute_loop_frame_index (block_index, audio)]);
      vector<float> out = synth_block (block);
      for (int i = -analysis_len; i < analysis_len; i++)
        if (x + i < int (synth_data.size()) && x + i >= 0)
          synth_data[x + i] += out[i + analysis_len];

      double fundamental_mag = 0;
      double fundamental_freq = 1;
      for (size_t i = 0; i < block.freqs.size(); i++)
        if ((block.freqs_f (i) - 1) < 1.25)
          {
            if (block.mags_f (i) > fundamental_mag)
              {
                fundamental_mag = block.mags_f (i);
                fundamental_freq = block.freqs_f (i);
              }
          }
      x += synthesis_len * fundamental_freq;
      block_index += synthesis_len / SR * (1000 / audio->frame_step_ms);
    }
  for (auto& s : synth_data)
    sm_printf ("%f\n", s);
}
