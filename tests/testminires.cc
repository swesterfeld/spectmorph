// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <string>
#include <vector>
#include "smfft.hh"
#include "smmain.hh"

#include <bse/bsemathsignal.hh>

using std::string;
using std::vector;

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

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  vector<float> audio_in (60000);
  for (size_t i = 0; i < audio_in.size(); i++)
    {
      audio_in[i] = sin (2 * M_PI * i * 24000.0 / 60000.);
    }
  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, 60000, 440, audio_in.size(), &audio_in[0], NULL);
  MiniResampler mini_resampler (dhandle, 60000. / 44100.);
  vector<float> out (44100);
  size_t FFT_SIZE = 32768;
  float *fft_in = FFT::new_array_float (FFT_SIZE);
  float *fft_out = FFT::new_array_float (FFT_SIZE);

  double normalize = 0;

  mini_resampler.read (0, out.size(), &out[0]);
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
}
