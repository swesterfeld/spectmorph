// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smwavset.hh"

#include <assert.h>

#include <map>

using namespace SpectMorph;

using std::min;
using std::map;
using std::string;

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

void
meta_comp (int& exit_code, const string& str, int old_value, int new_value)
{
  if (old_value != new_value)
    {
      printf ("# %s diff: old=%d new=%d\n", str.c_str(), old_value, new_value);
      exit_code = 1;
    }
}

void
meta_comp (int& exit_code, const string& str, const string& old_value, const string& new_value)
{
  if (old_value != new_value)
    {
      printf ("# %s diff: old='%s' new='%s'\n", str.c_str(), old_value.c_str(), new_value.c_str());
      exit_code = 1;
    }
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

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
  int fuzzy = argc >= 5 ? atoi (argv[4]) : 0;

  Audio *audio_old = nullptr;

  for (auto& w : smset_old.waves)
    if (w.midi_note == midi_note)
      audio_old = w.audio;

  Audio *audio_new = nullptr;

  for (auto& w : smset_new.waves)
    if (w.midi_note == midi_note)
      audio_new = w.audio;

  int exit_code = 0;

  meta_comp (exit_code, "name",       smset_old.name,       smset_new.name);
  meta_comp (exit_code, "short_name", smset_old.short_name, smset_new.short_name);

  if (audio_old && audio_new)
    {
      meta_comp (exit_code, "loop_type", audio_old->loop_type, audio_new->loop_type);
      if (audio_old->loop_type != 0 && audio_new->loop_type != 0)
        {
          meta_comp (exit_code, "loop_start", audio_old->loop_start,  audio_new->loop_start);
          meta_comp (exit_code, "loop_end",   audio_old->loop_end,    audio_new->loop_end);
        }
      meta_comp (exit_code, "zeropad",                audio_old->zeropad,               audio_new->zeropad);
      meta_comp (exit_code, "zero_values_at_start",   audio_old->zero_values_at_start,  audio_new->zero_values_at_start);
      meta_comp (exit_code, "frame_count",            audio_old->contents.size(),       audio_new->contents.size());
      meta_comp (exit_code, "sample_count",           audio_old->sample_count,          audio_new->sample_count);

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
              auto fuzzy_cmp = [fuzzy] (int a, int b) {
                return std::abs (a - b) > fuzzy;
              };

              bool id = true;

              for (;;)
                {
                  int best_diff = 100;
                  size_t best_j = 0, best_k = 0;
                  for (size_t j = 0; j < block_a.freqs.size(); j++)
                    {
                      for (size_t k = 0; k < block_b.freqs.size(); k++)
                        {
                          int of = block_a.freqs[j];
                          int nf = block_b.freqs[k];
                          if (of && nf && std::abs (of - nf) < best_diff)
                            {
                              best_diff = std::abs (of - nf);
                              best_j = j;
                              best_k = k;
                            }
                        }
                    }
                  if (best_diff < 100)
                    {
                      if (fuzzy_cmp (block_a.freqs[best_j], block_b.freqs[best_k]))
                        {
                          print_block();
                          sm_printf ("FFF  %zd %f %f %f\n", best_j, audio_old->contents[i].freqs_f (best_j), audio_new->contents[i].freqs_f (best_k),
                                                            audio_old->contents[i].freqs_f (best_j) / audio_new->contents[i].freqs_f (best_k));
                          id = false;
                        }
                      if (fuzzy_cmp (block_a.mags[best_j], block_b.mags[best_k]))
                        {
                          print_block();
                          sm_printf ("MMM  %zd %f %f %f # %f\n", best_j, block_a.mags_f (best_j), block_b.mags_f (best_k),
                                                                         block_a.mags_f (best_j) / block_b.mags_f (best_k),
                                                                         audio_old->contents[i].freqs_f (best_j) * audio_old->fundamental_freq);
                          id = false;
                        }
                      block_a.freqs[best_j] = 0;
                      block_b.freqs[best_k] = 0;
                    }
                  else
                    {
                      break;
                    }
                }
              auto dump_extra = [] (const char *str, auto block) {
                for (size_t j = 0; j < block.freqs.size(); j++)
                  {
                    if (block.freqs[j])
                      sm_printf ("%s  %zd %f %f\n", str, j, block.freqs_f (j), block.mags_f (j));
                  }
              };
              dump_extra ("OLD", block_a);
              dump_extra ("NEW", block_b);

              for (size_t j = 0; j < audio_old->contents[i].noise.size(); j++)
                {
                  if (fuzzy_cmp (block_a.noise[j], block_b.noise[j]))
                    {
                      print_block();
                      sm_printf ("NNN  %zd %f %f %f %d\n", j, block_a.noise_f (j), block_b.noise_f (j),
                                                           block_a.noise_f (j) / block_b.noise_f (j),
                                                           block_a.noise [j] - block_b.noise[j]);
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
