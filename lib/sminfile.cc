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

InFile::Event
InFile::event()
{
  if (current_event == NONE)
    next_event();

  return current_event;
}

string
InFile::read_raw_string()
{
  string s;
  s.reserve (64); // in normal files, this limit will almost never be exceeded

  int c;
  while ((c = fgetc (file)) > 0)
    s += c;
  return s;
}

void
InFile::next_event()
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
  else if (c == 's')
    {
      current_event = STRING;
      current_event_str  = read_raw_string();
      current_event_data = read_raw_string();
    }
  else if (c == 'F')
    {
      current_event = FLOAT_BLOCK;
      current_event_str = read_raw_string();

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
  a = fgetc (file);
  b = fgetc (file);
  c = fgetc (file);
  d = fgetc (file);
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

  fread (&buffer[0], 1, fb.size() * 4, file);
  for (size_t x = 0; x < fb.size(); x++)
    buffer[x] = GINT32_FROM_BE (buffer[x]);
}

void
InFile::skip_raw_float_block()
{
  size_t size = read_raw_int();
  fseek (file, size * 4, SEEK_CUR);
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
