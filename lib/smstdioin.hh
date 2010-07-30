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

#include <string>
#include "smgenericin.hh"

namespace SpectMorph
{

class StdioIn : public GenericIn
{
  FILE *file;
  StdioIn (FILE *file);
public:
  static GenericIn* open (const std::string& filename);

  int get_byte();     // like fgetc
  int read (void *ptr, size_t size);
  int seek (long offset, int whence);
  unsigned char *mmap_mem (size_t& remaining);
};

}
