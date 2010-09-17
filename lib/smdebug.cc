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


#include "smdebug.hh"
#include <stdio.h>
#include <stdarg.h>
#include <set>

using namespace SpectMorph;

using std::string;
using std::set;

static set<string> active_areas;
static FILE       *debug_file = NULL;

void
Debug::debug (const string& area, const char *fmt, ...)
{
  if (active_areas.find (area) != active_areas.end())
    {
      if (!debug_file)
        debug_file = fopen ("/tmp/smdebug.log", "w");

      va_list ap;

      va_start (ap, fmt);
      vfprintf (debug_file, fmt, ap);
      va_end (ap);
    }
}

void
Debug::debug_enable (const std::string& area)
{
  active_areas.insert (area);
}
