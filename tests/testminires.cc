// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <string>
#include <vector>
#include "smfft.hh"
#include "smmain.hh"
#include "smpolyphaseinter.hh"

#include <bse/bsemathsignal.hh>

using std::string;
using std::vector;
using std::max;

using namespace SpectMorph;

/// @cond
struct Options
{
  string	      program_name; /* FIXME: what to do with that */
  bool                full;

  Options () {}
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

void
scan (int high_sr, int sr, bool use_ppi)
{
  printf ("# scan: high_sr=%d, sr=%d, use_ppi=%s\n", high_sr, sr, use_ppi ? "true" : "false");

  for (double freq = 100; freq < (sr/2.0 - 1.0); freq += 123.456789)
    {
      vector<float> audio_in (high_sr);
      for (size_t i = 0; i < audio_in.size(); i++)
        {
          audio_in[i] = sin (2 * M_PI * i * freq / high_sr);
        }
      vector<float> out (sr);

      if (use_ppi)
        {
          PolyPhaseInter *ppi = PolyPhaseInter::the();
          for (size_t i = 0; i < out.size(); i++)
            {
              out[i] = ppi->get_sample (audio_in, double (high_sr) / double (sr) * i);
            }
        }
      else
        {
          GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, high_sr, 440, audio_in.size(), &audio_in[0], NULL);
          MiniResampler mini_resampler (dhandle, double (high_sr) / sr);
          mini_resampler.read (0, out.size(), &out[0]);
        }

      double max_diff = 0;
      for (size_t i = 100; i < out.size() - 100; i++)
        {
          double expect = sin (2 * M_PI * i * freq / sr);
          //printf ("%zd %.17g %.17g\n", i, out[i], expect);
          max_diff = max (max_diff, fabs (out[i] - expect));
        }
      double max_diff_db = 20 * log (max_diff) / log (10);
      printf ("%.17g %.17g\n", freq, max_diff_db);
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc == 4 && strcmp (argv[1], "scan") == 0)
    {
      scan (atoi (argv[2]), atoi (argv[3]), false);
    }
  else if (argc == 4 && strcmp (argv[1], "scanpp") == 0)
    {
      scan (atoi (argv[2]), atoi (argv[3]), true);
    }

#if 0
  size_t FFT_SIZE = 32768;
  float *fft_in = FFT::new_array_float (FFT_SIZE);
  float *fft_out = FFT::new_array_float (FFT_SIZE);

  double normalize = 0;

  for (int i = 0; i < FFT_SIZE; i++)
    {
      const double w = bse_window_blackman ((2.0 * i - FFT_SIZE) / FFT_SIZE);

      normalize += w;
      fft_in[i] = out[i] * w;
    }
  normalize *= 0.5;

  FFT::fftar_float (FFT_SIZE, fft_in, fft_out);

  for (guint i = 0; i < FFT_SIZE/2; i++)
    {
      const double normalized_error = bse_complex_abs (bse_complex (fft_out[i * 2], fft_out[i * 2 + 1])) / normalize;
      const double normalized_error_db = 20 * log (normalized_error) / log (10);

      printf ("%f %f\n", i / double (FFT_SIZE) * 44100, normalized_error_db);
    }

  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
#endif
}
