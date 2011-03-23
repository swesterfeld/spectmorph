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

#include "smstdioout.hh"
#include "smleakdebugger.hh"
#include <stdio.h>

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::StdioOut");

GenericOut*
StdioOut::open (const std::string& filename)
{
  FILE *file = fopen (filename.c_str(), "w");

  if (file)
    return new StdioOut (file);
  else
    return NULL;
}

StdioOut::StdioOut (FILE *file)
  : file (file)
{
  leak_debugger.add (this);
}

StdioOut::~StdioOut()
{
  if (file != NULL)
    {
      fclose (file);
      file = NULL;
    }
  leak_debugger.del (this);
}

int
StdioOut::put_byte (int c)
{
  return fputc (c, file);
}

int
StdioOut::write (const void *ptr, size_t size)
{
  return fwrite (ptr, 1, size, file);
}
