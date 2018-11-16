// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <string>
#include <vector>
#include "smfft.hh"
#include "smmain.hh"
#include "smpolyphaseinter.hh"
#include "smminiresampler.hh"
#include "smmath.hh"
#include <assert.h>
#include <math.h>
#include <string.h>

using std::string;
using std::vector;
using std::max;

using namespace SpectMorph;

void
get_error_signal (int high_sr, int sr, double freq, bool use_ppi, vector<float>& xout)
{
  vector<float> audio_in (high_sr), out (sr);
  for (size_t i = 0; i < audio_in.size(); i++)
    {
      audio_in[i] = sin (2 * M_PI * i * freq / high_sr);
    }

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
      WavData wav_data (audio_in, 1, high_sr, 32);
      MiniResampler mini_resampler (wav_data, double (high_sr) / sr);
      mini_resampler.read (0, out.size(), &out[0]);
    }
  assert (xout.size() == 0);
  for (size_t i = 100; i < out.size() - 100; i++)
    {
      double expect = sin (2 * M_PI * i * freq / sr);
      xout.push_back (out[i] - expect);
    }
}

void
scan (int high_sr, int sr, bool use_ppi)
{
  printf ("# scan: high_sr=%d, sr=%d, use_ppi=%s\n", high_sr, sr, use_ppi ? "true" : "false");

  for (double freq = 100; freq < (sr/2.0 - 1.0); freq += 100)
    {
      vector<float> out;
      get_error_signal (high_sr, sr, freq, use_ppi, out);

      double max_diff = 0;
      for (size_t i = 0; i < out.size(); i++)
        {
          max_diff = max (max_diff, double (out[i]));
        }
      double max_diff_db = 20 * log (max_diff) / log (10);
      printf ("%.17g %.17g\n", freq, max_diff_db);
    }
}

double
complex_abs (double re, double im)
{
  return sqrt (re * re + im * im);
}

void
error_spectrum (int high_sr, int sr, double freq, bool use_ppi)
{
  vector<float> out;
  get_error_signal (high_sr, sr, freq, use_ppi, out);

  size_t FFT_SIZE = 32768;
  float *fft_in = FFT::new_array_float (FFT_SIZE);
  float *fft_out = FFT::new_array_float (FFT_SIZE);

  double normalize = 0;

  for (guint i = 0; i < FFT_SIZE; i++)
    {
      const double w = window_blackman ((2.0 * i - FFT_SIZE) / FFT_SIZE);

      normalize += w;
      fft_in[i] = out[i] * w;
    }
  normalize *= 0.5;

  FFT::fftar_float (FFT_SIZE, fft_in, fft_out);

  for (guint i = 0; i < FFT_SIZE/2; i++)
    {
      const double normalized_error = complex_abs (fft_out[i * 2], fft_out[i * 2 + 1]) / normalize;
      const double normalized_error_db = 20 * log (normalized_error) / log (10);

      printf ("%f %f\n", i / double (FFT_SIZE) * 44100, normalized_error_db);
    }

  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc == 4 && strcmp (argv[1], "scan") == 0)
    {
      scan (atoi (argv[2]), atoi (argv[3]), false);
    }
  else if (argc == 4 && strcmp (argv[1], "scan-pp") == 0)
    {
      scan (atoi (argv[2]), atoi (argv[3]), true);
    }
  else if (argc == 5 && strcmp (argv[1], "error-spectrum") == 0)
    {
      error_spectrum (atoi (argv[2]), atoi (argv[3]), sm_atof (argv[4]), false);
    }
  else if (argc == 5 && strcmp (argv[1], "error-spectrum-pp") == 0)
    {
      error_spectrum (atoi (argv[2]), atoi (argv[3]), sm_atof (argv[4]), true);
    }
}
