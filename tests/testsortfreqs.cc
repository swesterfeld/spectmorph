// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>
#include <assert.h>

#include "smaudio.hh"
#include "smmain.hh"
#include "smmath.hh"

#include <algorithm>
#include <random>

using namespace SpectMorph;
using std::vector;

struct PData
{
  uint16_t freq;
  uint16_t mag;
};

float
something()
{
  return g_random_double_range (0, 0.1);
}

void
randomize_and_check (AudioBlock& block, std::mt19937& rand_gen)
{
  vector<PData> partials;

  for (size_t i = 0; i < 30; i++)
    {
      PData pd;
      pd.freq = sm_freq2ifreq (i + something());
      pd.mag = sm_factor2idb (i * 0.1 + something());

      partials.push_back (pd);
    }
  vector<PData> partials_shuffle = partials;
  std::shuffle (partials_shuffle.begin(), partials_shuffle.end(), rand_gen);

  assert (partials.size() == partials_shuffle.size());

  block.freqs.clear();
  block.mags.clear();
  block.phases.clear();

  for (size_t i = 0; i < partials.size(); i++)
    {
      block.freqs.push_back (partials_shuffle[i].freq);
      block.mags.push_back (partials_shuffle[i].mag);
    }

  AudioBlock check_block = block;
  check_block.sort_freqs();
  for (size_t i = 0; i < check_block.freqs.size(); i++)
    {
      assert (check_block.freqs[i] == partials[i].freq);
      assert (check_block.mags[i] == partials[i].mag);
    }
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  AudioBlock block_a, block_b, block_c;

  std::random_device rd;
  std::mt19937 rand_gen (rd());

  randomize_and_check (block_a, rand_gen);
  randomize_and_check (block_b, rand_gen);

  const unsigned int runs = 1000000;

  // assignments are not cheap, so we measure them seperately and subtract the result
  double xstart = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      block_c = block_a;
      block_c = block_b;
    }
  double xend = get_time();
  double xtime = xend - xstart;

  printf ("%.2f assign_blocks/sec\n", runs / 2 / xtime);

  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      block_c = block_a;
      block_c.sort_freqs();
      block_c = block_b;
      block_c.sort_freqs();
    }
  double end = get_time();
  printf ("%.2f sort_freqs/sec\n", runs / 2 / (end - start - xtime));
}
