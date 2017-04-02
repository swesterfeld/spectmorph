// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bse/gsldatahandle.hh>
#include <bse/gsldatautils.hh>
#include <errno.h>
#include <unistd.h>
#include "smmain.hh"
#include "smwavdata.hh"

using std::string;
using std::vector;
using SpectMorph::sm_init;
using SpectMorph::WavData;
using SpectMorph::Error;

int
main (int argc, char **argv)
{
  char buffer[1024];
  vector<float> signal;

  if (argc != 3)
    {
      printf ("usage example:\n\n");
      printf ("cat waveform.txt | ascii2wav wavform.wav 96000\n");
      exit (1);
    }
  /* init */
  sm_init (&argc, &argv);

  string filename = argv[1];
  int    srate    = atoi (argv[2]);

  int line = 1;
  while (fgets (buffer, 1024, stdin) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          char *end_ptr;
          bool parse_error = false;

          signal.push_back (g_ascii_strtod (buffer, &end_ptr));

          // chech that we parsed at least one character
          parse_error = parse_error || (end_ptr == buffer);

          // check that we parsed the whole line
          while (*end_ptr == ' ' || *end_ptr == '\n' || *end_ptr == '\t' || *end_ptr == '\r')
            end_ptr++;
          parse_error = parse_error || (*end_ptr != 0);

          if (parse_error)
            {
              g_printerr ("ascii2wav: parse error on line %d\n", line);
              exit (1);
            }
        }
      line++;
    }
  WavData wav_data (signal, 1, srate);

  if (!wav_data.save (filename))
    {
      g_printerr ("export to file %s failed: %s", filename.c_str(), wav_data.error_blurb());
      exit (1);
    }
}
