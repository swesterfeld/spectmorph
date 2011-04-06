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

#include <bse/bseloader.h>
#include <bse/gslfft.h>
#include <bse/bsemathsignal.h>
#include <bse/gsldatautils.h>

#include "smmain.hh"
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>

using std::string;
using std::vector;
using std::max;
using SpectMorph::sm_init;

/// @cond
struct Options
{
  string program_name;

  Options();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage();

  double signal_threshold;
  double silence_threshold;
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("imiscutter"),
  signal_threshold (-20),
  silence_threshold (-60)
{
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  g_return_if_fail (argc >= 0);

  /*  I am tired of seeing .libs/lt-gst123 all the time,
   *  but basically this should be done (to allow renaming the binary):
   *
  if (argc && argv[0])
    program_name = argv[0];
  */

  for (i = 1; i < argc; i++)
    {
      const char *opt_arg;
      if (strcmp (argv[i], "--help") == 0 ||
          strcmp (argv[i], "-h") == 0)
	{
	  print_usage();
	  exit (0);
	}
      else if (strcmp (argv[i], "--version") == 0 || strcmp (argv[i], "-v") == 0)
	{
	  printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--silence", &opt_arg))
	{
	  silence_threshold = atof (opt_arg);
	}
      else if (check_arg (argc, argv, &i, "--signal", &opt_arg))
        {
          signal_threshold = atof (opt_arg);
        }
    }

  /* resort argc/argv */
  e = 1;
  for (i = 1; i < argc; i++)
    if (argv[i])
      {
        argv[e++] = argv[i];
        if (i >= e)
          argv[i] = NULL;
      }
  *argc_p = e;
}

void
Options::print_usage ()
{
  printf ("usage: %s audiofile.wav <region-count> <first-region> <export-wav-pattern>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" --silence <threshold_db>    set silence threshold (default: %f)\n", options.silence_threshold);
  printf (" --signal <threshold_db>     set signal threshold (default: %f)\n", options.signal_threshold);
  printf ("\n");
}

static void
dump_wav (string filename, const vector<float>& sample, double mix_freq, int n_channels)
{
  GslDataHandle *out_dhandle = gsl_data_handle_new_mem (n_channels, 32, mix_freq, 44100 / 16 * 2048, sample.size(), &sample[0], NULL);
  BseErrorType error = gsl_data_handle_open (out_dhandle);
  if (error)
    {
      fprintf (stderr, "can not open mem dhandle for exporting wave file\n");
      exit (1);
    }

  int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0)
    {
      BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }
  int xerrno = gsl_data_handle_dump_wav (out_dhandle, fd, 16, out_dhandle->setup.n_channels, (guint) out_dhandle->setup.mix_freq);
  if (xerrno)
    {
      BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
    }
}

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

BseErrorType
read_dhandle (GslDataHandle *dhandle, vector<float>& signal)
{
  signal.clear();

  vector<float> block (1024);

  const uint64 n_values = gsl_data_handle_length (dhandle);
  uint64 pos = 0;
  while (pos < n_values)
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);
      if (r > 0)
        signal.insert (signal.end(), block.begin(), block.begin() + r);
      else
        return BSE_ERROR_FILE_READ_FAILED;
      pos += r;
    }
  return BSE_ERROR_NONE;
}

static void
compute_peaks (int channel, int note_len, const vector<float>& input_data, vector<double>& peaks, size_t block_size, int n_channels)
{
  peaks.clear();

  const size_t nl = note_len;

  for (size_t offset = channel; offset < input_data.size(); offset += block_size * n_channels)
    {
      vector<float> block;

      for (size_t i = 0; i < nl; i++)
        {
          size_t pos = offset + i * n_channels;
          if (pos < input_data.size())
            block.push_back (input_data[pos]);
          else
            block.push_back (0);
        }

      size_t fft_size = 1;
      while (fft_size < block.size() * 4)
        fft_size *= 2;

      vector<double> out (fft_size + 2);
      vector<double> in (fft_size);

      // produce fft-size periodic signal via linear interpolation from block-size periodic signal
      for (size_t in_pos = 0; in_pos < fft_size; in_pos++)
        {
          double pos = in_pos;

          pos /= fft_size;
          pos *= block.size();

          int ipos = pos;
          double dpos = pos - ipos;

          double left = block[ipos % block.size()];
          double right = block[(ipos + 1) % block.size()];

          in[in_pos] = left * (1.0 - dpos) + right * dpos;
        }

      gsl_power2_fftar (fft_size, &in[0], &out[0]);
      out[fft_size] = out[1];
      out[fft_size + 1] = 0;
      out[1] = 0;

      /* find peak */
      double peak = 0;
      for (uint64 t = 2; t < in.size(); t += 2)  // ignore DC
        {
          double a = out[t];
          double b = out[t+1];
          peak = max (peak, sqrt (a * a + b * b));
        }
      peaks.push_back (peak);
    }
  // normalize peaks with the biggest peak

  double max_peak = 0;
  for (size_t i = 0; i < peaks.size(); i++)
    max_peak = max (max_peak, peaks[i]);

  for (size_t i = 0; i < peaks.size(); i++)
    {
      peaks[i] = bse_db_from_factor (peaks[i] / max_peak, -500);
      //printf ("%.17g\n", peaks[i]);
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);

  if (argc != 5)
    {
      options.print_usage();
      exit (1);
    }

  /* open input */
  BseErrorType error;

  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (argv[1], &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], bse_error_blurb (error));
      exit (1);
    }

  const int n_channels = gsl_data_handle_n_channels (dhandle);
  const int mix_freq = gsl_data_handle_mix_freq (dhandle);

  const int region_count = atoi (argv[2]);
  const int first_region = atoi (argv[3]);

  vector<float> input_data;
  vector<double> peaks;

  error = read_dhandle (dhandle, input_data);
  if (error)
    {
      printf ("error reading input file %s: %s\n", argv[1], bse_error_blurb (error));
      exit (1);
    }

  const size_t block_size = 256;
  int last_region_end = 0;
  for (int region = first_region; region < first_region + region_count; region++)
    {
      int note_len = 0.5 + mix_freq / freq_from_note (region);

      peaks.clear();
      for (int channel = 0; channel < n_channels; channel++)
        {
          vector<double> channel_peaks;
          compute_peaks (channel, note_len, input_data, channel_peaks, block_size, n_channels);
          peaks.resize (channel_peaks.size(), -500);
          for (size_t peak = 0; peak < channel_peaks.size(); peak++)
            peaks[peak] = max (peaks[peak], channel_peaks[peak]);
        }

      size_t pi = last_region_end;
      while (pi < peaks.size() && peaks[pi] < options.signal_threshold)
        pi++;

      // search backwards for region start
      int start_pi = pi;
      while (start_pi > 0 && peaks[start_pi] > options.silence_threshold)
        start_pi--;

      // search forwards for region end
      size_t end_pi = pi;
      while (end_pi < peaks.size() && peaks[end_pi] > options.silence_threshold)
        end_pi++;

      last_region_end = end_pi;
      //printf ("%d %d %d\n", region, start_pi, end_pi);
      vector<float> sample (input_data.begin() + start_pi * block_size * n_channels,
                            input_data.begin() + end_pi * block_size * n_channels);

      char buffer[64];
      sprintf (buffer, argv[4], region);
      dump_wav (buffer, sample, mix_freq, n_channels);
      sample.clear();
    }
}
