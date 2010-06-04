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
      read_raw_float_block (current_event_float_block);
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


