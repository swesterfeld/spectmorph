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

#include "smmmapin.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

using SpectMorph::MMapIn;
using SpectMorph::GenericIn;

GenericIn*
MMapIn::open (const std::string& filename)
{
  if (getenv ("SPECTMORPH_NOMMAP"))
    return NULL;

  int fd = ::open (filename.c_str(), O_RDONLY);
  if (fd >= 0)
    {
      struct stat st;

      if (fstat (fd, &st) == 0)
        {
          unsigned char *mapfile = static_cast<unsigned char *> (mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
          if (mapfile != MAP_FAILED)
            {
              return new MMapIn (mapfile, mapfile + st.st_size, fd);
            }
        }
    }
  return NULL;
}

GenericIn*
MMapIn::open_mem (unsigned char *begin, unsigned char *end)
{
  return new MMapIn (begin, end, -1);
}

MMapIn::MMapIn (unsigned char *mapfile, unsigned char *mapend, int fd) :
  mapfile (mapfile),
  mapend (mapend),
  fd (fd)
{
  pos = static_cast<unsigned char *> (mapfile);
}

MMapIn::~MMapIn()
{
  if (fd >= 0)
    {
      munmap (mapfile, mapend - mapfile);
      close (fd);
    }
}

int
MMapIn::get_byte()
{
  if (pos < mapend)
    return *pos++;
  else
    return EOF;
}

int
MMapIn::read (void *ptr, size_t size)
{
  if (pos + size <= mapend)
    {
      memcpy (ptr, pos, size);
      pos += size;
      return size;
    }
  else
    return 0;
}

bool
MMapIn::skip (size_t size)
{
  if (pos + size <= mapend)
    {
      pos += size;
      return true;
    }
  else
    {
      return false;
    }
}

unsigned char *
MMapIn::mmap_mem (size_t& remaining)
{
  remaining = mapend - pos;
  return pos;
}

size_t
MMapIn::get_pos()
{
  return pos - mapfile;
}

GenericIn *
MMapIn::open_subfile (size_t pos, size_t len)
{
  return new MMapIn (mapfile + pos, mapfile + pos + len, -1 /* no fd */);
}
