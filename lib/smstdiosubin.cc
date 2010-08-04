/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smstdiosubin.hh"

#include <stdio.h>
#include <assert.h>

using SpectMorph::StdioSubIn;
using SpectMorph::GenericIn;

GenericIn*
StdioSubIn::open (const std::string& filename, size_t pos, size_t len)
{
  FILE *file = fopen (filename.c_str(), "r");

  if (file)
    return new StdioSubIn (file, pos, len);
  else
    return NULL;
}

StdioSubIn::StdioSubIn (FILE *file, size_t pos, size_t len) :
  file (file)
{
  fseek (file, pos, SEEK_SET);
  file_pos = 0;
  file_len = len;
}

int
StdioSubIn::get_byte()
{
  if (file_pos < file_len)
    {
      int result = fgetc (file);
      file_pos++;
      return result;
    }
  else
    {
      return EOF;
    }
}

int
StdioSubIn::read (void *ptr, size_t size)
{
  if (file_pos < file_len)
    {
      int read_bytes = fread (ptr, 1, size, file);
      file_pos += read_bytes;
      return read_bytes;
    }
  else
    {
      return EOF;
    }
}

bool
StdioSubIn::skip (size_t size)
{
  if (file_pos + size <= file_len)
    {
      if (fseek (file, size, SEEK_CUR) == 0)
        {
          file_pos += size;
          return true;
        }
    }
  return false;
}

unsigned char*
StdioSubIn::mmap_mem (size_t& remaining)
{
  return NULL;
}

size_t
StdioSubIn::get_pos()
{
  return file_pos;
}

GenericIn *
StdioSubIn::open_subfile (size_t pos, size_t len)
{
  assert (false); // implement me
  return 0;
}
