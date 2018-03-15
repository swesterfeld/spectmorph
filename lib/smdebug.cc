// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smdebug.hh"
#include "smutils.hh"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <set>
#include <mutex>
#include <atomic>

using namespace SpectMorph;

using std::string;
using std::set;

static set<string> active_areas;
static FILE       *debug_file = NULL;
static string      debug_filename = "smdebug.log";
static std::mutex  debug_mutex;
static std::atomic<int>  have_areas {0};

void
Debug::debug (const char *area, const char *fmt, ...)
{
  // no debugging -> return as quickly as possible
  if (!have_areas.load())
    return;

  std::lock_guard<std::mutex> locker (debug_mutex);
  if (active_areas.find (area) != active_areas.end())
    {
      if (!debug_file)
        {
          char *abs_filename = g_build_filename (g_get_tmp_dir(), debug_filename.c_str(), nullptr);
          debug_file = fopen (abs_filename, "w");
          g_free (abs_filename);
        }

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", string_vprintf (fmt, ap).c_str());
      va_end (ap);

      // hack: avoid fflush for encoder (since it writes lots of lines)
      if (strcmp (area, "encoder") != 0)
        fflush (debug_file);
    }
}

void
Debug::enable (const std::string& area)
{
  std::lock_guard<std::mutex> locker (debug_mutex);

  active_areas.insert (area);
  have_areas.store (1);
}

void
Debug::set_filename (const std::string& filename)
{
  std::lock_guard<std::mutex> locker (debug_mutex);

  debug_filename = filename;
}
