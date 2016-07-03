// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smsimplejackplayer.hh"
#include "smwavloader.hh"

#include <stdio.h>
#include <string.h>

using namespace SpectMorph;

using std::string;

void
command_loop()
{
  SimpleJackPlayer jack_player ("smevalplayer");
  jack_player.set_volume (1);

  char buffer[1024], *c;

  while (c = fgets (buffer, 1024, stdin))
    {
      char *cmd = strtok (buffer, " \n");
      if (!cmd)
        continue;

      char *arg = strtok (NULL, " \n");

      printf ("<%s>|<%s>\n", cmd, arg);

      if (strcmp (cmd, "play") == 0 && arg != 0)
        {
          string filename = arg;

          WavLoader *wav_loader = WavLoader::load (filename);
          if (wav_loader)
            {
              Audio audio;
              audio.mix_freq = wav_loader->mix_freq();
              audio.fundamental_freq = 440; /* doesn't matter */
              audio.original_samples = wav_loader->samples();

              jack_player.fade_out_blocking();
              jack_player.play (&audio, true);

              delete wav_loader;
            }
        }
      if (strcmp (cmd, "stop") == 0 && !arg)
        {
          jack_player.fade_out_blocking();
        }
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  command_loop();
  return 0;
}
