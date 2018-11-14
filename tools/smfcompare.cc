// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smwavset.hh"

#include <assert.h>

#include <map>

using namespace SpectMorph;

using std::min;
using std::map;

int
get_best_norm (Audio& audio_a, Audio& audio_b)
{
  map<int, double> delta_map;

  for (size_t b = 0; b < min (audio_a.contents.size(), audio_b.contents.size()); b++)
    {
      const AudioBlock& block_a = audio_a.contents[b];
      const AudioBlock& block_b = audio_b.contents[b];

      size_t min_size = min (block_a.mags.size(), block_b.mags.size());

      for (size_t i = 0; i < min_size; i++)
        {
          if (fabs (block_a.freqs_f (i) - block_b.freqs_f (i)) < 0.5)
            delta_map[int (block_a.mags[i]) - int (block_b.mags[i])] += min (block_a.mags_f (i), block_b.mags_f (i));
        }

      assert (block_a.noise.size() == block_b.noise.size());
      for (size_t i = 0; i < block_a.noise.size(); i++)
        {
          delta_map[int (block_a.noise[i]) - int (block_b.noise[i])] += min (block_a.noise_f (i), block_b.noise_f (i));
        }
    }
  int result = 0;
  double result_count = 0;

  for (auto d : delta_map)
    {
      if (d.second > result_count)
        {
          result = d.first;
          result_count = d.second;
        }
    }
  return result;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  WavSet smset_old;
  if (smset_old.load (argv[1]) != 0)
    {
      fprintf (stderr, "can't load file %s\n", argv[1]);
      return 1;
    }

  WavSet smset_new;
  if (smset_new.load (argv[2]) != 0)
    {
      fprintf (stderr, "can't load file %s\n", argv[2]);
      return 1;
    }
  int midi_note = atoi (argv[3]);

  Audio *audio_old = nullptr;

  for (auto& w : smset_old.waves)
    if (w.midi_note == midi_note)
      audio_old = w.audio;

  Audio *audio_new = nullptr;

  for (auto& w : smset_new.waves)
    if (w.midi_note == midi_note)
      audio_new = w.audio;

  int exit_code = 0;

  if (audio_old && audio_new)
    {
      if (audio_old->contents.size() == audio_new->contents.size())
        {
          size_t id_count = 0; // stats

          int norm = get_best_norm (*audio_old, *audio_new);
          sm_printf ("# global normalization: %d\n", norm);

          for (size_t i = 0; i < audio_old->contents.size(); i++)
            {
              AudioBlock block_a = audio_old->contents[i];
              AudioBlock block_b = audio_new->contents[i];
              if (abs (norm) < 1000)
                {
                  for (auto& v : block_b.noise)
                    v += norm;
                  for (auto& v : block_b.mags)
                    v += norm;
                }
              bool block_printed = false;
              auto print_block = [&]() {
                if (!block_printed)
                  {
                    block_printed = true;
                    sm_printf ("===== BLOCK %zd ===== %zd %zd =====\n", i, audio_old->contents[i].freqs.size(), audio_new->contents[i].freqs.size());
                  }
              };

              bool id = true;

              for (size_t j = 0; j < audio_old->contents[i].freqs.size(); j++)
                {
                  if (block_a.freqs[j] != block_b.freqs[j])
                    {
                      print_block();
                      sm_printf ("FFF  %zd %f %f %f\n", j, audio_old->contents[i].freqs_f (j), audio_new->contents[i].freqs_f(j),
                                                      audio_old->contents[i].freqs_f (j) / audio_new->contents[i].freqs_f(j));
                      id = false;
                    }
                  if (block_a.mags[j] != block_b.mags[j])
                    {
                      print_block();
                      sm_printf ("MMM  %zd %f %f %f # %f\n", j, block_a.mags_f (j), block_b.mags_f(j), block_a.mags_f (j) / block_b.mags_f(j),
                                                        audio_old->contents[i].freqs_f (j) * audio_old->fundamental_freq);
                      id = false;
                    }
                }
              for (size_t j = 0; j < audio_old->contents[i].noise.size(); j++)
                {
                  if (block_a.noise[j] != block_b.noise[j])
                    {
                      print_block();
                      sm_printf ("NNN  %zd %f %f %f\n", j, block_a.noise_f (j), block_b.noise_f(j),
                                                           block_a.noise_f (j) / block_b.noise_f(j));
                      id = false;
                    }
                }
              if (id)
                id_count++;
              else
                exit_code = 1;
            }
          printf ("# identical blocks: %zd/%zd\n", id_count, audio_new->contents.size());
        }
      else
        {
          printf ("# FAIL: different length: %zd vs. %zd blocks\n", audio_old->contents.size(), audio_new->contents.size());
          exit_code = 1;
        }
    }
  else
    {
      printf ("# FAIL: note not found old=%p  new=%p\n", audio_old, audio_new);
      exit_code = 1;
    }
  return exit_code;
}
