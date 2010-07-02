/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smaudio.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <bse/bse.h>

using std::string;
using std::vector;

/**
 * This function loads a SM-File.
 *
 * \param filename the name of the SM-File to be loaded
 * \returns a BseErrorType indicating whether loading was successful
 */
BseErrorType
SpectMorph::Audio::load (const string& filename, bool load_debug_info)
{
  SpectMorph::AudioBlock *audio_block = NULL;

  InFile  ifile (filename);
  string section;

  if (!ifile.open_ok())
    return BSE_ERROR_FILE_NOT_FOUND;

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          if (section == "frame")
            {
              assert (audio_block == NULL);
              audio_block = new SpectMorph::AudioBlock();
            }
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          if (section == "frame")
            {
              assert (audio_block);

              contents.push_back (*audio_block);
              delete audio_block;
              audio_block = NULL;
            }

          assert (section != "");
          section = "";
        }
      else if (ifile.event() == InFile::INT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "zeropad")
                zeropad = ifile.event_int();
              else
                printf ("unhandled int %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "mix_freq")
                mix_freq = ifile.event_float();
              else if (ifile.event_name() == "frame_size_ms")
                frame_size_ms = ifile.event_float();
              else if (ifile.event_name() == "frame_step_ms")
                frame_step_ms = ifile.event_float();
              else if (ifile.event_name() == "attack_start_ms")
                attack_start_ms = ifile.event_float();
              else if (ifile.event_name() == "attack_end_ms")
                attack_end_ms = ifile.event_float();
              else if (ifile.event_name() == "fundamental_freq")
                fundamental_freq = ifile.event_float();
              else
                printf ("unhandled float %s  %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::FLOAT_BLOCK)
        {
          const vector<float>& fb = ifile.event_float_block();

          assert (audio_block != NULL);
          if (ifile.event_name() == "noise")
            {
              audio_block->noise.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->noise.begin());
            }
          else if (ifile.event_name() == "freqs")
            {
              audio_block->freqs.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->freqs.begin());
            }
          else if (ifile.event_name() == "phases")
            {
              audio_block->phases.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->phases.begin());
            }
          else if (ifile.event_name() == "original_fft")
            {
              if (load_debug_info)
                {
                  audio_block->original_fft.resize (fb.size());
                  std::copy (fb.begin(), fb.end(), audio_block->original_fft.begin());
                }
            }
          else if (ifile.event_name() == "debug_samples")
            {
              if (load_debug_info)
                {
                  audio_block->debug_samples.resize (fb.size());
                  std::copy (fb.begin(), fb.end(), audio_block->debug_samples.begin());
                }
            }
          else
            {
              printf ("unhandled fblock %s %s\n", section.c_str(), ifile.event_name().c_str());
              assert (false);
            }
        }
      ifile.next_event();
    }
  return BSE_ERROR_NONE;
}


/**
 * This function saves a SM-File.
 *
 * \param filename the name of the SM-File to be written
 * \param audio the audio object to be stored
 * \returns a BseErrorType indicating saving loading was successful
 */
BseErrorType
SpectMorph::Audio::save (const string& filename)
{
  OutFile of (filename.c_str());
  if (!of.open_ok())
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", filename.c_str());
      exit (1);
    }

  of.begin_section ("header");
  of.write_float ("mix_freq", mix_freq);
  of.write_float ("frame_size_ms", frame_size_ms);
  of.write_float ("frame_step_ms", frame_step_ms);
  of.write_float ("attack_start_ms", attack_start_ms);
  of.write_float ("attack_end_ms", attack_end_ms);
  of.write_float ("fundamental_freq", fundamental_freq);
  of.write_int ("zeropad", zeropad);
  of.end_section();

  for (size_t i = 0; i < contents.size(); i++)
    {
      of.begin_section ("frame");
      of.write_float_block ("noise", contents[i].noise);
      of.write_float_block ("freqs", contents[i].freqs);
      of.write_float_block ("phases", contents[i].phases);
      of.write_float_block ("original_fft", contents[i].original_fft);
      of.write_float_block ("debug_samples", contents[i].debug_samples);
      of.end_section();
    }
  return BSE_ERROR_NONE;
}
