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

#ifndef SPECTMORPH_GENERIC_OUT_HH
#define SPECTMORPH_GENERIC_OUT_HH

#include <string>

namespace SpectMorph
{

class GenericOut
{
public:
  virtual ~GenericOut();

  virtual int put_byte (int c) = 0;     // like fputc
  virtual int write (const void *ptr, size_t size) = 0;
};

}

#endif /* SPECTMORPH_GENERIC_OUT_HH */
