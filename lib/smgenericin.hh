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

#ifndef SPECTMORPH_GENERIC_IN_HH
#define SPECTMORPH_GENERIC_IN_HH

#include <string>

namespace SpectMorph
{

class GenericIn
{
public:
  virtual int get_byte() = 0;     // like fgetc
  virtual int read (void *ptr, size_t size) = 0;
  virtual int seek (long offset, int whence) = 0;
  virtual unsigned char *mmap_mem (size_t& remaining) = 0;
};

}

#endif /* SPECTMORPH_GENERIC_IN_HH */
