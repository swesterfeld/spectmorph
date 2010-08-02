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

#include "sminfile.hh"
#include <assert.h>
#include <glib.h>

using std::string;
using std::vector;
using SpectMorph::InFile;
using SpectMorph::GenericIn;

InFile::InFile (const string& filename)
{
  file = MMapIn::open (filename);
  current_event = NONE;

  read_file_type();
}

InFile::InFile (GenericIn *file)
  : file (file)
{
  current_event = NONE;
  read_file_type();
}

void
InFile::read_file_type()
{
  m_file_type = "unknown";

  if (file)
    {
      if (file->get_byte() == 'T')
        read_raw_string (m_file_type);
    }
}

InFile::Event
InFile::event()
{
  if (current_event == NONE)
    next_event();

  return current_event;
}

void
InFile::read_raw_string (string& str)
{
  size_t remaining;
  unsigned char *mem = file->mmap_mem (remaining);
  if (mem) /* fast variant of reading strings for the mmap case */
    {
      for (size_t i = 0; i < remaining; i++)
        {
          if (mem[i] == 0)
            {
              file->seek (i + 1, SEEK_CUR);
              str.assign (reinterpret_cast <char *> (mem), i);
              return;
            }
        }
    }

  str.clear();

  int c;
  while ((c = file->get_byte()) > 0)
    str += c;
}

void
InFile::next_event()
{
  int c = file->get_byte();

  if (c == EOF)
    {
      current_event = END_OF_FILE;
      return;
    }
  else if (c == 'B')
    {
      current_event = BEGIN_SECTION;
      read_raw_string (current_event_str);
    }
  else if (c == 'E')
    {
      current_event = END_SECTION;
    }
  else if (c == 'f')
    {
      current_event = FLOAT;
      read_raw_string (current_event_str);
      current_event_float = read_raw_float();
    }
  else if (c == 'i')
    {
      current_event = INT;
      read_raw_string (current_event_str);
      current_event_int = read_raw_int();
    }
  else if (c == 's')
    {
      current_event = STRING;
      read_raw_string (current_event_str);
      read_raw_string (current_event_data);
    }
  else if (c == 'F')
    {
      current_event = FLOAT_BLOCK;
      read_raw_string (current_event_str);

      if (skip_events.find (current_event_str) != skip_events.end())
        {
          skip_raw_float_block();
          return next_event();
        }
      else
        {
          read_raw_float_block (current_event_float_block);
        }
    }
  else if (c == 'O')
    {
      current_event = BLOB;
      read_raw_string (current_event_str);
      current_event_blob_size = read_raw_int();
      current_event_blob_pos  = file->get_pos();

      // skip actual blob data
      file->seek (current_event_blob_size, SEEK_CUR);
    }
  else
    {
      printf ("unhandled char '%c'\n", c);
      assert (false);
    }
}

int
InFile::read_raw_int()
{
  int a, b, c, d;
  a = file->get_byte();
  b = file->get_byte();
  c = file->get_byte();
  d = file->get_byte();
  return ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff);
}

float
InFile::read_raw_float()
{
  union {
    float f;
    int i;
  } u;
  u.i = read_raw_int();
  return u.f;
}

void
InFile::read_raw_float_block (vector<float>& fb)
{
  size_t size = read_raw_int();

  fb.resize (size);
  int *buffer = reinterpret_cast <int*> (&fb[0]);

  file->read (&buffer[0], fb.size() * 4);
  for (size_t x = 0; x < fb.size(); x++)
    buffer[x] = GINT32_FROM_BE (buffer[x]);
}

void
InFile::skip_raw_float_block()
{
  size_t size = read_raw_int();
  file->seek (size * 4, SEEK_CUR);
}

GenericIn *
InFile::open_blob()
{
  return file->open_subfile (current_event_blob_pos, current_event_blob_size);
}

string
InFile::event_name()
{
  return current_event_str;
}

float
InFile::event_float()
{
  return current_event_float;
}

int
InFile::event_int()
{
  return current_event_int;
}

string
InFile::event_data()
{
  return current_event_data;
}

const vector<float>&
InFile::event_float_block()
{
  return current_event_float_block;
}

void
InFile::add_skip_event (const string& skip_event)
{
  skip_events.insert (skip_event);
}

string
InFile::file_type()
{
  return m_file_type;
}
