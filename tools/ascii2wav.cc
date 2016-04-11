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

using std::string;
using std::vector;
using SpectMorph::sm_init;

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
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, srate, 440, signal.size(), &signal[0], NULL);
  gsl_data_handle_open (dhandle);

  int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    {
      Bse::Error error = bse_error_from_errno (errno, Bse::Error::FILE_OPEN_FAILED);
      g_printerr ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  int xerrno = gsl_data_handle_dump_wav (dhandle, fd, 16, dhandle->setup.n_channels, (guint) dhandle->setup.mix_freq);
  if (xerrno)
    {
      Bse::Error error = bse_error_from_errno (xerrno, Bse::Error::FILE_WRITE_FAILED);
      g_printerr ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
      exit (1);
    }
  close (fd);
}
