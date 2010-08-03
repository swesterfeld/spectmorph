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
#include "smstdioout.hh"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <bse/bse.h>

using std::string;
using std::vector;

using SpectMorph::GenericIn;
using SpectMorph::GenericOut;
using SpectMorph::StdioOut;

/**
 * This function loads a SM-File.
 *
 * \param filename the name of the SM-File to be loaded
 * \param load_options specify whether to load or skip debug information
 * \returns a BseErrorType indicating whether loading was successful
 */
BseErrorType
SpectMorph::Audio::load (const string& filename, AudioLoadOptions load_options)
{
  GenericIn *file = GenericIn::open (filename);
  return load (file, load_options);
}

BseErrorType
SpectMorph::Audio::load (GenericIn *file, AudioLoadOptions load_options)
{
  SpectMorph::AudioBlock *audio_block = NULL;

  InFile ifile (file);

  string section;
  size_t contents_pos;

  if (!ifile.open_ok())
    return BSE_ERROR_FILE_NOT_FOUND;

  if (ifile.file_type() != "SpectMorph::Audio")
    return BSE_ERROR_FORMAT_INVALID;

  if (load_options == AUDIO_SKIP_DEBUG)
    {
      ifile.add_skip_event ("original_fft");
      ifile.add_skip_event ("debug_samples");
    }

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          if (section == "frame")
            {
              assert (audio_block == NULL);
              assert (contents_pos < contents.size());

              audio_block = &contents[contents_pos];
            }
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          if (section == "frame")
            {
              assert (audio_block);

              contents_pos++;
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
              else if (ifile.event_name() == "loop_point")
                loop_point = ifile.event_int();
              else if (ifile.event_name() == "zero_values_at_start")
                zero_values_at_start = ifile.event_int();
              else if (ifile.event_name() == "frame_count")
                {
                  int frame_count = ifile.event_int();

                  contents.clear();
                  contents.resize (frame_count);
                  contents_pos = 0;
                }
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
              audio_block->noise = fb;
            }
          else if (ifile.event_name() == "freqs")
            {
              audio_block->freqs = fb;
            }
          else if (ifile.event_name() == "phases")
            {
              audio_block->phases = fb;
            }
          else if (ifile.event_name() == "original_fft")
            {
              audio_block->original_fft = fb;
            }
          else if (ifile.event_name() == "debug_samples")
            {
              audio_block->debug_samples = fb;
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


SpectMorph::Audio::Audio() :
  zeropad (0),
  loop_point (-1) /* no loop */
{
}

/**
 * This function saves a SM-File.
 *
 * \param filename the name of the SM-File to be written
 * \returns a BseErrorType indicating saving loading was successful
 */
BseErrorType
SpectMorph::Audio::save (const string& filename)
{
  GenericOut *out = StdioOut::open (filename);
  if (!out)
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", filename.c_str());
      exit (1);
    }
  BseErrorType result = save (out);
  delete out; // close file

  return result;
}

BseErrorType
SpectMorph::Audio::save (GenericOut *file)
{
  OutFile of (file, "SpectMorph::Audio");
  assert (of.open_ok());

  of.begin_section ("header");
  of.write_float ("mix_freq", mix_freq);
  of.write_float ("frame_size_ms", frame_size_ms);
  of.write_float ("frame_step_ms", frame_step_ms);
  of.write_float ("attack_start_ms", attack_start_ms);
  of.write_float ("attack_end_ms", attack_end_ms);
  of.write_float ("fundamental_freq", fundamental_freq);
  of.write_int ("zeropad", zeropad);
  of.write_int ("loop_point", loop_point);
  of.write_int ("zero_values_at_start", zero_values_at_start);
  of.write_int ("frame_count", contents.size());
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
