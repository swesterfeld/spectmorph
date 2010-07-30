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

#include "smstdioin.hh"

#include <stdio.h>

using SpectMorph::StdioIn;
using SpectMorph::GenericIn;

GenericIn*
StdioIn::open (const std::string& filename)
{
  FILE *file = fopen (filename.c_str(), "r");

  if (file)
    return new StdioIn (file);
  else
    return NULL;
}

StdioIn::StdioIn (FILE *file) :
  file (file)
{
}

int
StdioIn::get_byte()
{
  return fgetc (file);
}

int
StdioIn::read (void *ptr, size_t size)
{
  fread (ptr, 1, size, file);
}

int
StdioIn::seek (long offset, int whence)
{
  fseek (file, offset, whence);
}
