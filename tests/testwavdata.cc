// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"

#include <assert.h>
#include <math.h>

using namespace SpectMorph;

using std::vector;
using std::string;

void
save_load_test (WavData::OutFormat format, string ext)
{
  vector<float> signal;

  for (int i = -0x8000; i <= 0x7FFF; i++)
    signal.push_back (i / double (0x8000));

  WavData wav_data (signal, 1, 48000, 16);
  wav_data.save ("testwd." + ext, format);

  WavData a;
  bool ok = a.load ("testwd." + ext);
  assert (ok);

  int repeat_error = 0;
  int reload_error = 0;

  double last_sample = -1000;
  for (auto sample : a.samples())
    {
      //printf ("%.10f\n", sample);
      if (sample == last_sample)
        {
          repeat_error++;
        }
      last_sample = sample;
    }
  a.save ("testwd2." + ext, format);
  WavData b;
  if (b.load ("testwd2." + ext))
    {
      assert (a.n_values() == b.n_values());
      for (size_t i = 0; i < a.n_values(); i++)
        {
          if (a[i] != b[i])
            reload_error++;
        }
    }
  printf ("save/load test: ERR: repeat = %d; load = %d\n", repeat_error, reload_error);
  assert (repeat_error == 0 && reload_error == 0);
}

void
clip_test (WavData::OutFormat format, string ext)
{
  vector<float> signal;
  signal.push_back (1 + 0.0001);
  signal.push_back (1 + 0.001);
  signal.push_back (1 + 0.01);
  signal.push_back (1 + 0.1);
  signal.push_back (2);
  signal.push_back (50);
  signal.push_back (-1 - 0.0001);
  signal.push_back (-1 - 0.001);
  signal.push_back (-1 - 0.01);
  signal.push_back (-1 - 0.1);
  signal.push_back (-2);
  signal.push_back (-50);

  WavData wav_data (signal, 1, 48000, 16);
  wav_data.save ("testwd." + ext, format);

  WavData a;
  bool ok = a.load ("testwd." + ext);
  assert (ok);

  for (auto sample : a.samples())
    printf ("%.10f\n", sample);
}

void
save_vec_test (WavData::OutFormat format, string ext)
{
  vector<float> signal;
  for (size_t i = 0; i < 48000; i++)
    signal.push_back (sin (i * 2 * M_PI * 440 / 48000));

  WavData wav_data (signal, 1, 48000, 16);

  vector<unsigned char> out;
  bool ok = wav_data.save (out, format);
  assert (ok);

  printf ("save ok: %s - size: %zd\n", ok ? "OK" : "FAIL", out.size());
  FILE *t = fopen (("testwd." + ext).c_str(), "w");
  fwrite (&out[0], 1, out.size(), t);
  fclose (t);

  WavData a;
  ok = a.load ("testwd." + ext);
  assert (ok);

  assert (signal.size() == a.n_values());
  for (size_t i = 0; i < a.n_values(); i++)
    {
      /* FIXME: strange: the difference of signal[i] and a[i] seems to be positive
       * all the time? why? shouldn't we sometimes round up and sometimes round down?
       */
      assert (fabs (signal[i] - a[i]) < 1 / 32000.);
    }

  WavData b;
  ok = b.load (out);
  assert (ok);

  for (size_t i = 0; i < b.n_values(); i++)
    assert (a[i] == b[i]);
}

void
run_tests (WavData::OutFormat format, string ext)
{
  save_load_test (format, ext);
  clip_test (format, ext);
  save_vec_test (format, ext);
}

int
main()
{
  run_tests (WavData::OutFormat::WAV, "wav");
  run_tests (WavData::OutFormat::FLAC, "flac");
}
