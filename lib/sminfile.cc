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
  file = GenericIn::open (filename);
  current_event = NONE;

  read_file_type_and_version();
}

InFile::InFile (GenericIn *file)
  : file (file)
{
  current_event = NONE;
  read_file_type_and_version();
}

void
InFile::read_file_type_and_version()
{
  if (file)
    {
      if (file->get_byte() == 'T')
        if (read_raw_string (m_file_type))
          if (file->get_byte() == 'V')
            if (read_raw_int (m_file_version))
              return;
    }
  m_file_type = "unknown";
  m_file_version = 0;
}

InFile::Event
InFile::event()
{
  if (current_event == NONE)
    next_event();

  return current_event;
}

bool
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
              if (file->skip (i + 1))
                {
                  str.assign (reinterpret_cast <char *> (mem), i);
                  return true;
                }
            }
        }
    }

  str.clear();

  int c;
  while ((c = file->get_byte()) > 0)
    str += c;

  if (c == 0)
    return true;
  return false;
}

void
InFile::next_event()
{
  int c = file->get_byte();

  if (c == 'Z')  // eof
    {
      if (file->get_byte() == EOF)   // Z needs to be followed by EOF
        current_event = END_OF_FILE;
      else                           // Z and more stuff is an error
        current_event = READ_ERROR;
      return;
    }
  else if (c == 'B')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        current_event = BEGIN_SECTION;
    }
  else if (c == 'E')
    {
      current_event = END_SECTION;
    }
  else if (c == 'f')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_float (current_event_float))
          current_event = FLOAT;
    }
  else if (c == 'i')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_int (current_event_int))
          current_event = INT;
    }
  else if (c == 's')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_string (current_event_data))
          current_event = STRING;
    }
  else if (c == 'F')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        {
          if (skip_events.find (current_event_str) != skip_events.end())
            {
              if (skip_raw_float_block())
                {
                  next_event();
                  return;
                }
            }
          else
            {
              if (read_raw_float_block (current_event_float_block))
                current_event = FLOAT_BLOCK;
            }
        }
    }
  else if (c == 'O')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        {
          int blob_size;
          if (read_raw_int (blob_size))
            {
              int blob_pos = file->get_pos();
              if (file->skip (blob_size)) // skip actual blob data
                {
                  current_event = BLOB;
                  current_event_blob_size = blob_size;
                  current_event_blob_pos  = blob_pos;
                }
            }
        }
    }
  else
    {
      current_event = READ_ERROR;
    }
}

bool
InFile::read_raw_int (int& i)
{
  if (file->read (&i, 4) == 4)
    {
      // little endian encoding
      i = GINT32_FROM_LE (i);
      return true;
    }
  else
    {
      return false;
    }
}

bool
InFile::read_raw_float (float &f)
{
  union {
    float f;
    int i;
  } u;
  bool result = read_raw_int (u.i);
  f = u.f;
  return result;
}

bool
InFile::read_raw_float_block (vector<float>& fb)
{
  int size;
  if (!read_raw_int (size))
    return false;

  fb.resize (size);
  int *buffer = reinterpret_cast <int*> (&fb[0]);

  if (file->read (&buffer[0], fb.size() * 4) != size * 4)
    return false;

#if G_BYTE_ORDER != G_LITTLE_ENDIAN
  for (size_t x = 0; x < fb.size(); x++)
    buffer[x] = GINT32_FROM_LE (buffer[x]);
#endif
  return true;
}

bool
InFile::skip_raw_float_block()
{
  int size;
  if (!read_raw_int (size))
    return false;

  return file->skip (size * 4);
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

int
InFile::file_version()
{
  return m_file_version;
}
