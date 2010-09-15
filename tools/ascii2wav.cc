/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bse/gsldatahandle.h>
#include <bse/gsldatautils.h>
#include <errno.h>
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

  while (fgets (buffer, 1024, stdin) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          signal.push_back (atof (buffer));
        }
    }
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, srate, 440, signal.size(), &signal[0], NULL);
  gsl_data_handle_open (dhandle);

  int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    {
      BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }

  int xerrno = gsl_data_handle_dump_wav (dhandle, fd, 16, dhandle->setup.n_channels, (guint) dhandle->setup.mix_freq);
  if (xerrno)
    {
      BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }
  close (fd);
}
