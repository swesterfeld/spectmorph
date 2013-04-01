// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

#include "smaudio.hh"
#include "smmain.hh"

using namespace SpectMorph;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

using namespace SpectMorph;

void
randomize_and_check (AudioBlock& block)
{
  vector<float> freqs;

  for (size_t i = 0; i < 30; i++)
    {
      freqs.push_back (i * 440);
    }
  vector<float> freqs_shuffle = freqs;
  std::random_shuffle (freqs_shuffle.begin(), freqs_shuffle.end());

  block.freqs = freqs_shuffle;
  block.mags.resize (block.freqs.size());
  block.phases.resize (block.freqs.size());

  AudioBlock check_block = block;
  check_block.sort_freqs();
  for (size_t i = 0; i < check_block.freqs.size(); i++)
    {
      assert (check_block.freqs[i] == freqs[i]);
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  AudioBlock block_a, block_b, block_c;

  randomize_and_check (block_a);
  randomize_and_check (block_b);

  const unsigned int runs = 1000000;
  double start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    {
      block_c = block_a;
      block_c.sort_freqs();
      block_c = block_b;
      block_c.sort_freqs();
    }
  double end = gettime();
  printf ("%.2f sort_freqs/sec\n", runs / 2 / (end - start));
}
