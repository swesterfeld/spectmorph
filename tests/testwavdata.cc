// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;

void
save_load_test()
{
  vector<float> signal;

  for (int i = -0x8000; i <= 0x7FFF; i++)
    signal.push_back (i / double (0x8000));

  WavData wav_data (signal, 1, 48000, 16);
  wav_data.save ("testwd.wav");

  WavData a;
  bool ok = a.load ("testwd.wav");
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
  a.save ("testwd2.wav");
  WavData b;
  if (b.load ("testwd2.wav"))
    {
      assert (a.n_values() == b.n_values());
      for (size_t i = 0; i < a.n_values(); i++)
        {
          if (a[i] != b[i])
            reload_error++;
        }
    }
  printf ("save/load test: ERR: repeat = %d; load = %d\n", repeat_error, reload_error);
}

void
clip_test()
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
  wav_data.save ("testwd.wav");

  WavData a;
  bool ok = a.load ("testwd.wav");
  assert (ok);

  for (auto sample : a.samples())
    printf ("%.10f\n", sample);
}

int
main()
{
  save_load_test();
  clip_test();
}
