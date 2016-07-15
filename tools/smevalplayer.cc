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
  bool debug = false;

  while (c = fgets (buffer, 1024, stdin))
    {
      char *cmd = strtok (buffer, " \n");
      if (!cmd)
        continue;

      char *arg = strtok (NULL, " \n");

      // only print filenames in debug mode (for double-blind tests)
      if (strcmp (cmd, "debug") == 0 && !arg)
        debug = true;

      if (debug)
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

              if (debug)
                {
                  bool need_resample = fabs (wav_loader->mix_freq() - jack_player.mix_freq()) > 1e-6;
                  printf ("  -> resample = %s\n", need_resample ? "true" : "false");
                }
              jack_player.play (&audio, true);

              delete wav_loader;
            }
        }
      if (strcmp (cmd, "stop") == 0 && !arg)
        {
          jack_player.stop();
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
