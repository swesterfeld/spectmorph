// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaudio.hh"
#include "smwavset.hh"
#include "smmain.hh"
#include "smlpc.hh"

#include <assert.h>

using std::vector;

using namespace SpectMorph;

double
gain_error (double gain, const Audio& audio, const AudioBlock& block, LPC::LSFEnvelope& env)
{
  double error = 0;

  for (size_t i = 0; i < block.freqs.size(); i++)
    {
      const double lsf_freq = block.freqs_f (i) * audio.fundamental_freq / LPC::MIX_FREQ * 2 * M_PI;
      const double delta = block.mags_f (i) - env.eval (lsf_freq) * gain;
      error += block.mags_f (i) * delta * delta;
    }
  return error;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  assert (argc == 3 || argc == 4);

  Audio *audio_ptr = nullptr;

  if (argc == 3)
    {
      static Audio audio;

      audio.load (argv[1]);

      audio_ptr = &audio;
    }
  else
    {
      static WavSet wset;

      wset.load (argv[1]);
      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          if (wi->midi_note == atoi (argv[2]))
            audio_ptr = wi->audio;
        }
      assert (audio_ptr);
    }
  const Audio& audio = *audio_ptr;

  const AudioBlock& block = audio.contents[atoi (argv[argc - 1])];

  LPC::LSFEnvelope env;
  env.init (block.lpc_lsf_p, block.lpc_lsf_q);

  /* fit gain */
  double gain = 1, gain_step = 1.01;
  while (gain_error (gain / gain_step, audio, block, env) < gain_error (gain, audio, block, env))
    gain /= gain_step;

  const size_t grid = 22050;
  for (size_t i = 0; i <= grid; i++)
    {
      double f = i * M_PI / grid;
      printf ("L %f %.17g\n", f / (2 * M_PI) * LPC::MIX_FREQ, env.eval (f) * gain);
    }

  for (size_t i = 0; i < block.freqs.size(); i++)
    {
      const double lsf_freq = block.freqs_f (i) * audio.fundamental_freq / LPC::MIX_FREQ * 2 * M_PI;
      printf ("F %f %.17g %.17g\n", block.freqs_f (i) * audio.fundamental_freq, block.mags_f (i), env.eval (lsf_freq) * gain);
    }
}
