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

#include <bse/bsemain.h>
#include <bse/bseloader.h>
#include <bse/gslfft.h>
#include <bse/bsemathsignal.h>
#include <bse/gsldatautils.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <vector>

using std::string;
using std::vector;
using std::max;

/// @cond
struct Options
{
  string program_name;
} options;
/// @endcond

static int
count_regions (const vector<double>& peaks, double silence_threshold)
{
  int   regions = 0;
  bool  in_region = false;

  for (vector<double>::const_iterator pi = peaks.begin(); pi != peaks.end(); pi++)
    {
      if (*pi > silence_threshold)
        {
          if (!in_region)
            {
              in_region = true;
              regions++;
            }
        }
      else
        {
          if (in_region)
            {
              in_region = false;
            }
        }
    }
  return regions;
}

static void
dump_wav (string filename, const vector<float>& sample, double mix_freq)
{
  GslDataHandle *out_dhandle = gsl_data_handle_new_mem (1, 32, mix_freq, 44100 / 16 * 2048, sample.size(), &sample[0], NULL);
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

int
main (int argc, char **argv)
{
  bse_init_inprocess (&argc, &argv, NULL, NULL);

  options.program_name = "imiscutter";

  if (argc != 5)
    {
      printf ("usage: %s audiofile.wav <region-count> <first-region> <export-wav-pattern>\n", options.program_name.c_str());
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

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }
  const int mix_freq = gsl_data_handle_mix_freq (dhandle);

  const int    region_count = atoi (argv[2]);
  const int    first_region = atoi (argv[3]);
  const size_t block_size = 256;

  vector<float> block (block_size);
  vector<float> input_data;
  vector<double> peaks;

  vector<double> window (block_size);
  window.resize (block_size);
  for (guint i = 0; i < window.size(); i++)
    window[i] = bse_window_cos (2.0 * i / block_size - 1.0);

  const uint64 n_values = gsl_data_handle_length (dhandle);
  for (uint64 pos = 0; pos < n_values; pos += block.size())
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      /* for short reads (EOF) fill the rest with zeros */
      std::fill (block.begin() + r, block.end(), 0);

      /* build input data vector (containing the whole file) */
      input_data.insert (input_data.end(), block.begin(), block.end());

      /* FFT windowed block */
      vector<double> in (block.begin(), block.end());
      vector<double> out (block_size + 2);

      for (uint64 t = 0; t < block.size(); t += 2)
        in[t] *= window[t];

      gsl_power2_fftar (block_size, &in[0], &out[0]);
      out[block_size] = out[1];
      out[block_size + 1] = 0;
      out[1] = 0;

      /* find peak */
      double peak = 0;
      for (uint64 t = 2; t < block.size(); t += 2)  // ignore DC
        {
          double a = out[t];
          double b = out[t+1];
          peak = max (peak, sqrt (a * a + b * b));
        }
      peaks.push_back (peak);
#if 0
      for (uint64 t = 0; t < block.size(); t++)
        printf ("%.17g %.17g\n", block[t], peak);
#endif
    }

  // normalize peaks with the biggest peak
  double max_peak = 0;
  for (size_t i = 0; i < peaks.size(); i++)
    max_peak = max (max_peak, peaks[i]);

  for (size_t i = 0; i < peaks.size(); i++)
    {
      peaks[i] = bse_db_from_factor (peaks[i] / max_peak, -500);
      // printf ("%.17g\n", peaks[i]);
    }


  double best_silence_threshold = 0, silence_threshold = 0;
  while (silence_threshold > -60)
    {
      if (count_regions (peaks, silence_threshold) == region_count
      &&  peaks.front() < silence_threshold && peaks.back() < silence_threshold)
        best_silence_threshold = silence_threshold;

      silence_threshold -= .1;
    }

  assert (best_silence_threshold < -5);

  bool  in_region = false;
  int   region_number = first_region;
  size_t region_start;
  for (size_t p = 0; p < peaks.size(); p++)
    {
      if (peaks[p] > best_silence_threshold)
        {
          if (!in_region)
            {
              in_region = true;
              region_start = p;
            }
        }
      else
        {
          if (in_region)
            {
              if (region_start > 0)   // include one extra block at the beginning to prevent clicks
                region_start--;

              size_t region_end = p + 1; // and one extra_block at the end

              vector<float> sample (input_data.begin() + region_start * block_size, input_data.begin() + region_end * block_size);

              char buffer[64];
              sprintf (buffer, argv[4], region_number++);
              dump_wav (buffer, sample, mix_freq);
              sample.clear();
              in_region = false;
            }
        }
    }
  printf ("%f\n", best_silence_threshold);
}
