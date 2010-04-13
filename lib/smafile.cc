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

#include "smafile.hh"
#include "smaudio.hh"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <bse/bse.h>

using std::string;
using std::vector;

class IFile
{
public:
  enum Event {
    NONE,
    END_OF_FILE,
    BEGIN_SECTION,
    END_SECTION,
    INT,
    FLOAT,
    FLOAT_BLOCK
  };

protected:
  FILE *file;
  Event         current_event;
  string        current_event_str;
  int           current_event_int;
  float         current_event_float;
  vector<float> current_event_float_block;

  string read_raw_string();
  int    read_raw_int();
  float  read_raw_float();
  void   read_raw_float_block (vector<float>& fb);

public:
  IFile (const string& file_name)
  {
    file = fopen (file_name.c_str(), "r");
    current_event = NONE;
  }
  bool
  open_ok()
  {
    return file != NULL;
  }
  Event event();
  string event_name();
  float  event_float();
  int    event_int();
  const vector<float>& event_float_block();
  void  next_event();
};

IFile::Event
IFile::event()
{
  if (current_event == NONE)
    next_event();

  return current_event;
}

string
IFile::read_raw_string()
{
  string s;
  int c;
  while ((c = fgetc (file)) > 0)
    s += c;
  return s;
}

void
IFile::next_event()
{
  int c = fgetc (file);

  if (c == EOF)
    {
      current_event = END_OF_FILE;
      return;
    }
  else if (c == 'B')
    {
      current_event = BEGIN_SECTION;
      current_event_str = read_raw_string();
    }
  else if (c == 'E')
    {
      current_event = END_SECTION;
    }
  else if (c == 'f')
    {
      current_event = FLOAT;
      current_event_str = read_raw_string();
      current_event_float = read_raw_float();
    }
  else if (c == 'i')
    {
      current_event = INT;
      current_event_str = read_raw_string();
      current_event_int = read_raw_int();
    }
  else if (c == 'F')
    {
      current_event = FLOAT_BLOCK;
      current_event_str = read_raw_string();
      read_raw_float_block (current_event_float_block);
    }
  else
    {
      printf ("unhandled char '%c'\n", c);
      assert (false);
    }
}

int
IFile::read_raw_int()
{
  int a, b, c, d;
  a = fgetc (file);
  b = fgetc (file);
  c = fgetc (file);
  d = fgetc (file);
  return ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff);
}

float
IFile::read_raw_float()
{
  union {
    float f;
    int i;
  } u;
  u.i = read_raw_int();
  return u.f;
}

void
IFile::read_raw_float_block (vector<float>& fb)
{
  size_t size = read_raw_int();

  vector<unsigned char> buffer (size * 4);
  fread (&buffer[0], 1, buffer.size(), file);

  fb.resize (size);
  for (size_t x = 0; x < fb.size(); x++)
    {
      // fb[x] = read_raw_float();
      union {
        float f;
        int   i;
      } u;
      int a = buffer[x * 4];
      int b = buffer[x * 4 + 1];
      int c = buffer[x * 4 + 2];
      int d = buffer[x * 4 + 3];
      u.i = ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff);
      fb[x] = u.f;
    }
}

string
IFile::event_name()
{
  return current_event_str;
}

float
IFile::event_float()
{
  return current_event_float;
}

int
IFile::event_int()
{
  return current_event_int;
}

const vector<float>&
IFile::event_float_block()
{
  return current_event_float_block;
}

BseErrorType
STWAFile::load (const string& file_name,
                SpectMorph::Audio& audio_out)
{
  SpectMorph::Audio audio;
  SpectMorph::AudioBlock *audio_block = NULL;

  IFile  ifile (file_name);
  string section;

  if (!ifile.open_ok())
    return BSE_ERROR_FILE_NOT_FOUND;

  while (ifile.event() != IFile::END_OF_FILE)
    {
      if (ifile.event() == IFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          if (section == "frame")
            {
              assert (audio_block == NULL);
              audio_block = new SpectMorph::AudioBlock();
            }
        }
      else if (ifile.event() == IFile::END_SECTION)
        {
          if (section == "frame")
            {
              assert (audio_block);

              audio.contents.push_back (*audio_block);
              delete audio_block;
              audio_block = NULL;
            }

          assert (section != "");
          section = "";
        }
      else if (ifile.event() == IFile::INT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "zeropad")
                audio.zeropad = ifile.event_int();
              else
                printf ("unhandled int %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == IFile::FLOAT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "mix_freq")
                audio.mix_freq = ifile.event_float();
              else if (ifile.event_name() == "frame_size_ms")
                audio.frame_size_ms = ifile.event_float();
              else if (ifile.event_name() == "frame_step_ms")
                audio.frame_step_ms = ifile.event_float();
              else if (ifile.event_name() == "fundamental_freq")
                audio.fundamental_freq = ifile.event_float();
              else
                printf ("unhandled float %s  %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == IFile::FLOAT_BLOCK)
        {
          const vector<float>& fb = ifile.event_float_block();

          assert (audio_block != NULL);
          if (ifile.event_name() == "meaning")
            {
              audio_block->meaning.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->meaning.begin());
            }
          else if (ifile.event_name() == "freqs")
            {
              audio_block->freqs.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->freqs.begin());
            }
          else if (ifile.event_name() == "mags")
            {
              audio_block->mags.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->mags.begin());
            }
          else if (ifile.event_name() == "phases")
            {
              audio_block->phases.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->phases.begin());
            }
          else if (ifile.event_name() == "original_fft")
            {
              audio_block->original_fft.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->original_fft.begin());
            }
          else if (ifile.event_name() == "debug_samples")
            {
              audio_block->debug_samples.resize (fb.size());
              std::copy (fb.begin(), fb.end(), audio_block->debug_samples.begin());
            }
          else
            {
              printf ("unhandled fblock %s %s\n", section.c_str(), ifile.event_name().c_str());
              assert (false);
            }
        }
      ifile.next_event();
    }
  audio_out = audio;
  return BSE_ERROR_NONE;
}

class OFile
{
  FILE *file;
protected:
  void write_raw_string (const string& s);
  void write_raw_int (int i);

public:
  OFile (const string& file_name)
  {
    file = fopen (file_name.c_str(), "w");
  }
  bool
  open_ok()
  {
    return file != NULL;
  }
  void begin_section (const string& s);
  void end_section();

  void write_int (const string& s, int i);
  void write_float (const string& s, double f);
  void write_float_block (const string& s, const vector<float>& fb);
};

void
OFile::begin_section (const string& s)
{
  fputc ('B', file); // begin section
  write_raw_string (s);
}

void
OFile::end_section()
{
  fputc ('E', file); // end section
}

void
OFile::write_raw_string (const string& s)
{
  for (size_t i = 0; i < s.size(); i++)
    fputc (s[i], file);
  fputc (0, file);
}

void
OFile::write_raw_int (int i)
{
  fputc ((i >> 24) & 0xff, file);
  fputc ((i >> 16) & 0xff, file);
  fputc ((i >> 8) & 0xff, file);
  fputc (i & 0xff, file);
}

void
OFile::write_float (const string& s,
                    double f)
{
  union {
    float f;
    int i;
  } u;
  u.f = f;

  fputc ('f', file);

  write_raw_string (s);
  write_raw_int (u.i);
}

void
OFile::write_int (const string& s,
                  int   i)
{
  fputc ('i', file);

  write_raw_string (s);
  write_raw_int (i);
}

void
OFile::write_float_block (const string& s,
                          const vector<float>& fb)
{
  fputc ('F', file);

  write_raw_string (s);
  write_raw_int (fb.size());

  vector<unsigned char> buffer (fb.size() * 4);
  size_t bpos = 0;
  for (size_t i = 0; i < fb.size(); i++)
    {
      union {
        float f;
        int i;
      } u;
      u.f = fb[i];
      // write_raw_int (u.i);
      buffer[bpos++] = u.i >> 24;
      buffer[bpos++] = u.i >> 16;
      buffer[bpos++] = u.i >> 8;
      buffer[bpos++] = u.i;
    }
  assert (bpos == buffer.size());
  fwrite (&buffer[0], 1, buffer.size(), file);
}

BseErrorType
STWAFile::save (const string& file_name,
                const SpectMorph::Audio& audio)
{
  OFile of (file_name.c_str());
  if (!of.open_ok())
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", file_name.c_str());
      exit (1);
    }

  of.begin_section ("header");
  of.write_float ("mix_freq", audio.mix_freq);
  of.write_float ("frame_size_ms", audio.frame_size_ms);
  of.write_float ("frame_step_ms", audio.frame_step_ms);
  of.write_float ("fundamental_freq", audio.fundamental_freq);
  of.write_int ("zeropad", audio.zeropad);
  of.end_section();

  for (size_t i = 0; i < audio.contents.size(); i++)
    {
      of.begin_section ("frame");
      of.write_float_block ("meaning", audio.contents[i].meaning);
      of.write_float_block ("freqs", audio.contents[i].freqs);
      of.write_float_block ("mags", audio.contents[i].mags);
      of.write_float_block ("phases", audio.contents[i].phases);
      of.write_float_block ("original_fft", audio.contents[i].original_fft);
      of.write_float_block ("debug_samples", audio.contents[i].debug_samples);
      of.end_section();
    }
  return BSE_ERROR_NONE;
}
