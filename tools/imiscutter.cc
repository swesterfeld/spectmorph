// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <bse/bseloader.hh>
#include <bse/gslfft.hh>
#include <bse/bsemathsignal.hh>
#include <bse/gsldatautils.hh>

#include "smmain.hh"
#include "smfft.hh"
#include "config.h"
#include "smwavdata.hh"
#include "smmath.hh"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include <string>
#include <vector>

using std::string;
using std::vector;
using std::max;
using std::min;

using namespace SpectMorph;

/// @cond
struct Options
{
  string program_name;

  Options();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage();

  double signal_threshold;
  double silence_threshold;
  int    step;
} options;
/// @endcond

#include "stwutils.hh"

Options::Options () :
  program_name ("imiscutter"),
  signal_threshold (-20),
  silence_threshold (-60),
  step (1)
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
      else if (check_arg (argc, argv, &i, "--step", &opt_arg))
        {
          step = atoi (opt_arg);
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

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

static void
compute_peaks (int channel, int note_len, const vector<float>& input_data, vector<double>& peaks, size_t block_size, int n_channels)
{
  peaks.clear();

  const size_t nl = note_len;
  vector<float> block (nl);

  size_t fft_size = 1;
  while (fft_size < nl * 4)
    fft_size *= 2;

  float *fft_in = FFT::new_array_float (fft_size);
  float *fft_out = FFT::new_array_float (fft_size + 2);

  for (size_t offset = channel; offset < input_data.size(); offset += block_size * n_channels)
    {
      size_t input_pos = offset;
      for (size_t i = 0; i < nl; i++)
        {
          if (input_pos < input_data.size())
            block[i] = input_data[input_pos];
          else
            block[i] = 0;
          input_pos += n_channels;
        }

      // produce fft-size periodic signal via linear interpolation from block-size periodic signal
      double pos = 0;
      double pos_inc = (1.0 / fft_size) * block.size();
      for (size_t in_pos = 0; in_pos < fft_size; in_pos++)
        {
          int ipos = pos;
          double dpos = pos - ipos;

          if (ipos + 1 < block.size())
            fft_in[in_pos] = block[ipos] * (1.0 - dpos) + block[ipos + 1] * dpos;
          else
            fft_in[in_pos] = block[ipos % block.size()] * (1.0 - dpos) + block[(ipos + 1) % block.size()] * dpos;
          pos += pos_inc;
        }

      FFT::fftar_float (fft_size, fft_in, fft_out);
      fft_out[fft_size] = fft_out[1];
      fft_out[fft_size + 1] = 0;
      fft_out[1] = 0;

      /* find peak (we search biggest squared peak, but thats the same) */
      double peak_2 = 0;
      for (size_t t = 2; t < fft_size; t += 2)  // ignore DC
        {
          const float a = fft_out[t];
          const float b = fft_out[t+1];
          const double a_2_b_2 = a * a + b * b;

          if (a_2_b_2 > peak_2)
            peak_2 = a_2_b_2;
        }
      double peak = sqrt (peak_2);
      peaks.push_back (peak);
    }
  // normalize peaks with the biggest peak

  double max_peak = 0;
  for (size_t i = 0; i < peaks.size(); i++)
    max_peak = max (max_peak, peaks[i]);

  for (size_t i = 0; i < peaks.size(); i++)
    {
      peaks[i] = db_from_factor (peaks[i] / max_peak, -500);
      //printf ("%.17g\n", peaks[i]);
    }
  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
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
  WavData wav_data;
  if (!wav_data.load (argv[1]))
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), argv[1], wav_data.error_blurb());
      exit (1);
    }
  const int n_channels            = wav_data.n_channels();
  const int mix_freq              = wav_data.mix_freq();
  const vector<float>& input_data = wav_data.samples();

  const int region_count = atoi (argv[2]);
  const int first_region = atoi (argv[3]);

  vector<double> peaks;

  const size_t block_size = 256;
  int last_region_end = 0;
  for (int region = first_region; region != first_region + region_count * options.step; region += options.step)
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
      const size_t start_pos = start_pi * block_size * n_channels;
      const size_t end_pos   = min (end_pi * block_size * n_channels, input_data.size());
      vector<float> sample (input_data.begin() + start_pos, input_data.begin() + end_pos);

      string out_filename = string_printf (argv[4], region);

      WavData wav_out_data (sample, n_channels, mix_freq);
      if (!wav_out_data.save (out_filename))
        {
          fprintf (stderr, "export to file %s failed: %s", out_filename.c_str(), wav_out_data.error_blurb());
          exit (1);
        }
      sample.clear();
    }
}
