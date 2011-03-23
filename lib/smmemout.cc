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

#include "smmemout.hh"
#include "smleakdebugger.hh"
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MemOut");

MemOut::MemOut (vector<unsigned char> *output)
  : output (output)
{
  leak_debugger.add (this);
}

MemOut::~MemOut()
{
  leak_debugger.del (this);
}

int
MemOut::put_byte (int c)
{
  output->push_back (c);
  return c;
}

int
MemOut::write (const void *ptr, size_t size)
{
  const unsigned char *uptr = reinterpret_cast<const unsigned char *> (ptr);
  output->insert (output->end(), uptr, uptr + size);
  return size;
}
