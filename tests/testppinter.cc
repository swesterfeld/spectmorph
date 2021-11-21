// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"

#include <vector>

#include <assert.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;
using std::min;

void
sin_test (double freq, double db_bound)
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();

  double step[] = { 0.456, 0.567, 0.888, 0.901, 1.01, 1.2, 1.3, -1 };
  float error = 0;

  for (int s = 0; step[s] > 0; s++)
    {
      const double SR = 48000;
      vector<float> input (SR * 2);
      vector<float> expect (SR);
      vector<float> output (SR);

      for (size_t t = 0; t < input.size(); t++)
        input[t] = sin (t * 2 * M_PI * freq / SR);

      for (size_t t = 0; t < output.size(); t++)
        {
          output[t] = ppi->get_sample (input, step[s] * t);
          expect[t] = sin (t * step[s] * 2 * M_PI * freq / SR);
        }

      for (size_t p = 20; p < output.size() - 20; p++)
        {
          error = max (error, fabs (output[p] - expect[p]));
        }
    }
  printf ("%.17g %.17g %.17g\n", freq, error, db_from_factor (error, -200));
  assert (error < db_to_factor (db_bound));
}

void
sweep_test()
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  const double SR = 48000;

  for (double freq = 5; freq < 24000; freq += 5)
    {
      size_t T = SR * 0.1; /* 100 ms */
      vector<float> signal (T);

      for (size_t i = 0; i < signal.size(); i++)
        signal[i] = sin (i * 2 * M_PI * freq / SR);

      const double SPEED = 4.71;

      double error = 0;
      for (double pos = 100; pos < signal.size() - 100; pos += SPEED)
        {
          double interp = ppi->get_sample (signal, pos);
          double expect = sin (pos * 2 * M_PI * freq / SR);

          error = max (error, fabs (interp - expect));
        }
      printf ("%.2f %.17g\n", freq, db_from_factor (error, -200));
    }
}

double
sinc (double x)
{
  if (fabs (x) < 1e-6)
    return 1;
  else
    return sin (M_PI * x) / (M_PI * x);
}

void
impulse_test()
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  vector<float> one_signal; // impulse signal
  one_signal.push_back (1);

  const int FFT_SIZE = 256 * 1024;
  float *fft_in = FFT::new_array_float (FFT_SIZE);
  float *fft_out = FFT::new_array_float (FFT_SIZE);
  int k = 0;

  const double SR = 48000;
  const double LP_FREQ = 20000;
  const double STEP = 0.001;
  for (double p = -20; p < 20; p += STEP)
    {
      double c = sinc (double (p) / (SR / 2) * LP_FREQ);
      double w = window_blackman (double (p) / 12) / (SR / 2) * LP_FREQ;
      double x = c * w;

      x = ppi->get_sample (one_signal, p);

      fft_in[k++] = x * STEP;
    }
  FFT::fftar_float (FFT_SIZE, fft_in, fft_out, FFT::PLAN_ESTIMATE);
  fft_out[1] = 0;
  for (int i = 0; i < FFT_SIZE; i += 2)
    {
      double re = fft_out[i];
      double im = fft_out[i + 1];
      const double freq = double (i / 2) / FFT_SIZE / STEP * 48000;

      printf ("%.3f %.17g\n", freq, sqrt (re * re + im * im));
    }
}

void
print_spectrum (const string& label, const vector<float>& signal, double SR)
{
  size_t input_size = SR;

  size_t fft_size = 1;
  while (fft_size < input_size * 4)
    fft_size *= 2;

  float *fft_in = FFT::new_array_float (fft_size);
  float *fft_out = FFT::new_array_float (fft_size);

  zero_float_block (fft_size, fft_in);

  double normalize = 0;
  for (size_t i = 0; i < input_size; i++)
    {
      double w = window_blackman (2.0 * i / input_size - 1.0);
      fft_in[i] = w * signal[i];
      normalize += w;
    }
  FFT::fftar_float (fft_size, fft_in, fft_out, FFT::PLAN_ESTIMATE);

  normalize *= 0.5;

  for (size_t i = 0; i < fft_size / 2; i++)
    {
      double re = fft_out[i * 2];
      double im = fft_out[i * 2 + 1];
      if (i == 0) // special packing
        im = 0;
      printf ("%f %.17g #%s\n", i / double (fft_size) * SR, sqrt (re * re + im * im) / normalize, label.c_str());
    }
}

void
resample_test (const double SR, const double FREQ, const double SPEED)
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();

  vector<float> input_signal (5 * SR);   // 5 seconds
  vector<float> expect_signal (5 * SR);  // 5 seconds

  // generate input signal, which should be band limited after resampling
  for (size_t i = 0; i < input_signal.size(); i++)
    {
      int partial = 1;
      while (partial * FREQ * SPEED < (SR / 2))
        {
          input_signal[i] += sin (partial * i / 48000.0 * FREQ * 2 * M_PI) / partial;
          partial++;
        }
    }

  // generate perfect output signal
  for (size_t i = 0; i < expect_signal.size(); i++)
    {
      int partial = 1;
      while (partial * FREQ < (SR / 2))
        {
          if (partial * SPEED * FREQ < (SR / 2))    // filter at nyquist
            expect_signal[i] += sin (partial * i / 48000.0 * FREQ * SPEED * 2 * M_PI) / partial;
          partial++;
        }
    }

  // generate resampled result
  vector<float> resampled_signal (5 * SR);
  for (size_t i = 0; i < resampled_signal.size(); i++)
    {
      double d = i * SPEED;
      //resampled_signal[i] = get_li (input_signal, d);
      resampled_signal[i] = ppi->get_sample (input_signal, d);
    }

  print_spectrum ("I", input_signal, SR);
  print_spectrum ("E", expect_signal, SR);
  print_spectrum ("R", resampled_signal, SR);
  //print_signal (expect_signal, SR);
  //print_signal (resampled_signal, SR);
}

void
speed_test()
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  const double SR = 48000;

  vector<float> signal (SR * 5);
  for (size_t i = 0; i < signal.size(); i++)
    {
      signal[i] = g_random_double_range (-1, 1);
    }

  double t[2];

  vector<float> result (SR);

  double start, end;

  start = get_time();
  const size_t RUNS = 300, PADDING = 16;
  for (size_t k = 0; k < RUNS; k++)
    for (size_t i = PADDING; i < result.size(); i++)
      result[i] = ppi->get_sample (signal, i * 0.987);
  end = get_time();
  t[1] = end - start;

  start = get_time();
  for (size_t k = 0; k < RUNS; k++)
    for (size_t i = PADDING; i < result.size(); i++)
      result[i] = ppi->get_sample_no_check (signal, i * 0.987);
  end = get_time();
  t[0] = end - start;

  for (int checks = 0; checks < 2; checks++)
    {
      double ns_per_sec = 1e9;
      double ns_per_sample = t[checks] * ns_per_sec / (RUNS * (result.size() - PADDING));
      printf (" ** checks = %d\n", checks);
      printf ("interp: %f ns/sample\n", ns_per_sample);
      printf ("bogopolyphony = %f\n", ns_per_sec / (ns_per_sample * 48000));
      printf ("\n");
    }
}

void
rspectrum (double freq, double speed)
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  const double SR = 48000;

  // generate source signal: sine wave
  vector<float> source (SR * speed + 100);
  for (size_t i = 0; i < source.size(); i++)
    source[i] = sin (i * freq / SR * 2 * M_PI);

  // generate output signal & expected output
  vector<float> dest, expect, delta;
  for (double pos = 0; pos < SR * speed; pos += speed)
    {
      const double dest_value = ppi->get_sample (source, pos + 50);
      const double expect_value = sin ((pos + 50) * freq / SR * 2 * M_PI);

      dest.push_back (dest_value);
      expect.push_back (expect_value);
      delta.push_back (dest_value - expect_value);

      //printf ("%.2f %.17g %.17g\n", pos, dest.back(), expect.back());
    }
  print_spectrum ("I", source, SR);
  print_spectrum ("E", expect, SR / speed);
  print_spectrum ("R", dest, SR / speed);
  print_spectrum ("D", delta, SR / speed);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  if (argc == 2 && string (argv[1]) == "sweep")
    {
      sweep_test();
    }
  else if (argc == 2 && string (argv[1]) == "impulse")
    {
      impulse_test();
    }
  else if (argc == 2 && string (argv[1]) == "speed")
    {
      speed_test();
    }
  else if (argc == 2 && string (argv[1]) == "resample")
    {
      resample_test (48000, 440, 1.3);
    }
  else if (argc == 4 && string (argv[1]) == "rspectrum")
    {
      rspectrum (sm_atof (argv[2]), sm_atof (argv[3]));
    }
  else
    {
      sin_test (440, -85);
      sin_test (2000, -75);
    }
}
