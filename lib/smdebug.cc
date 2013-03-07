// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
Debug::debug (const char *area, const char *fmt, ...)
{
  // no debugging -> return as quickly as possible
  if (active_areas.empty())
    return;

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
