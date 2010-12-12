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

#include "smwavset.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include "smmemout.hh"

#include <map>
#include <set>

#include <assert.h>

using std::vector;
using std::string;
using std::map;
using std::set;

using SpectMorph::WavSet;
using SpectMorph::WavSetWave;
using SpectMorph::MemOut;

BseErrorType
WavSet::save (const string& filename, bool embed_models)
{
  OutFile of (filename.c_str(), "SpectMorph::WavSet", SPECTMORPH_BINARY_FILE_VERSION);
  if (!of.open_ok())
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", filename.c_str());
      exit (1);
    }

  for (size_t i = 0; i < waves.size(); i++)
    {
      of.begin_section ("wave");
      of.write_int ("midi_note", waves[i].midi_note);
      of.write_int ("channel", waves[i].channel);
      of.write_int ("velocity_range_min", waves[i].velocity_range_min);
      of.write_int ("velocity_range_max", waves[i].velocity_range_max);
      of.write_string ("path", waves[i].path);
      if (waves[i].audio)
        {
          vector<unsigned char> data;

          MemOut mem_out (&data);
          waves[i].audio->save (&mem_out);
          of.write_blob ("audio", &data[0], data.size());
        }
      else if (embed_models)
        {
          FILE *in = fopen (waves[i].path.c_str(), "r");
          if (in)
            {
              vector<unsigned char> data;
              vector<unsigned char> buffer (1024);
              size_t len;

              do
                {
                  len = fread (&buffer[0], 1, buffer.size(), in);
                  if (len > 0)
                    data.insert (data.end(), buffer.begin(), buffer.begin() + len);
                }
              while (len > 0);

              if (ferror (in))
                {
                  fprintf (stderr, "wavset save: error reading file: %s\n", waves[i].path.c_str());
                }
              else
                {
                  of.write_blob ("audio", &data[0], data.size());
                }
              fclose (in);
            }
          else
            {
              fprintf (stderr, "wavset save: missing file: %s\n", waves[i].path.c_str());
            }
        }
      of.end_section();
    }
  return BSE_ERROR_NONE;
}

BseErrorType
WavSet::load (const string& filename)
{
  clear();        // delete old contents (if any)

  map<string, Audio *> blob_map;

  WavSetWave *wave = NULL;

  InFile  ifile (filename);
  string section;

  if (!ifile.open_ok())
    return BSE_ERROR_FILE_NOT_FOUND;

  if (ifile.file_type() != "SpectMorph::WavSet")
    return BSE_ERROR_FORMAT_INVALID;

  if (ifile.file_version() != SPECTMORPH_BINARY_FILE_VERSION)
    return BSE_ERROR_FORMAT_INVALID;

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          if (section == "wave")
            {
              assert (wave == NULL);
              wave = new WavSetWave();
            }
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          if (section == "wave")
            {
              assert (wave);

              waves.push_back (*wave);
              delete wave;
              wave = NULL;
            }

          assert (section != "");
          section = "";
        }
      else if (ifile.event() == InFile::INT)
        {
          if (section == "wave")
            {
              if (ifile.event_name() == "midi_note")
                {
                  assert (wave);
                  wave->midi_note = ifile.event_int();
                }
              else if (ifile.event_name() == "channel")
                {
                  assert (wave);
                  wave->channel = ifile.event_int();
                }
              else if (ifile.event_name() == "velocity_range_min")
                {
                  assert (wave);
                  wave->velocity_range_min = ifile.event_int();
                }
              else if (ifile.event_name() == "velocity_range_max")
                {
                  assert (wave);
                  wave->velocity_range_max = ifile.event_int();
                }
              else
                printf ("unhandled int %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::STRING)
        {
          if (section == "wave")
            {
              if (ifile.event_name() == "path")
                {
                  assert (wave);
                  wave->path = ifile.event_data();
                }
              else
                printf ("unhandled string %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::BLOB)
        {
          if (section == "wave")
            {
              if (ifile.event_name() == "audio")
                {
                  assert (wave);
                  assert (!wave->audio);

                  wave->audio = new Audio();
                  wave->audio->load (ifile.open_blob());

                  blob_map[ifile.event_blob_sum()] = wave->audio;
                }
              else
                printf ("unhandled string %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::BLOB_REF)
        {
          if (section == "wave")
            {
              if (ifile.event_name() == "audio")
                {
                  assert (wave);
                  assert (!wave->audio);

                  wave->audio = blob_map[ifile.event_blob_sum()];

                  assert (wave->audio);
                }
              else
                printf ("unhandled string %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          return BSE_ERROR_PARSE_ERROR;
        }
      else
        {
          assert (false);
        }
      ifile.next_event();
    }
  return BSE_ERROR_NONE;
}

WavSetWave::WavSetWave()
{
  audio = NULL;
  channel = 0;
  midi_note = -1;
  velocity_range_min = 0;
  velocity_range_max = 127;
}

WavSetWave::~WavSetWave()
{
  // delete audio; // <- don't do it here, because of the constructor/destructor calls in the vector
}

void
WavSet::clear()
{
  set<Audio *> to_delete;

  for (vector<WavSetWave>::iterator wi = waves.begin(); wi != waves.end(); wi++)
    {
      if (wi->audio)
        {
          to_delete.insert (wi->audio);
          wi->audio = NULL;
        }
    }

  /* This is a little piece of extra logic that prevents deleting one Audio entry twice;
     the same entry can be reused in one WavSet multiple times (for instance for different
     velocity levels) - and while BLOB_REFs care for reusing the same object in this case,
     we need to ensure here we don't delete the same object twice */
  for (set<Audio *>::const_iterator di = to_delete.begin(); di != to_delete.end(); di++)
    delete *di;

  // now that everything has been delete-d, we can reset the waves vector
  waves.clear();
}

WavSet::~WavSet()
{
  clear();
}
