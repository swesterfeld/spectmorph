// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smdebug.hh"
#include "smutils.hh"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <glib.h>
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

static inline void
debugv (const char *area, const char *format, va_list vargs)
{
  std::lock_guard<std::mutex> locker (debug_mutex);
  if (active_areas.find (area) != active_areas.end())
    {
      if (!debug_file)
        {
          char *abs_filename = g_build_filename (sm_get_user_dir (USER_DIR_DATA).c_str(), debug_filename.c_str(), nullptr);
          debug_file = fopen (abs_filename, "w");
          g_free (abs_filename);
        }

      fprintf (debug_file, "%s", string_vprintf (format, vargs).c_str());

      // hack: avoid fflush for encoder (since it writes lots of lines)
      if (strcmp (area, "encoder") != 0)
        fflush (debug_file);
    }
}

namespace SpectMorph
{

void
Debug::debug (const char *area, const char *fmt, ...)
{
  // no debugging -> return as quickly as possible
  if (!have_areas.load())
    return;

  va_list ap;

  va_start (ap, fmt);
  debugv (area, fmt, ap);
  va_end (ap);
}

void
sm_debug (const char *fmt, ...)
{
  // no debugging -> return as quickly as possible
  if (!have_areas.load())
    return;

  va_list ap;

  va_start (ap, fmt);
  debugv ("global", fmt, ap);
  va_end (ap);
}

void
Debug::enable (const std::string& area)
{
  std::lock_guard<std::mutex> locker (debug_mutex);

  active_areas.insert (area);
  have_areas.store (1);
}

bool
Debug::enabled (const std::string& area)
{
  std::lock_guard<std::mutex> locker (debug_mutex);

  return active_areas.find (area) != active_areas.end();
}

void
Debug::set_filename (const std::string& filename)
{
  std::lock_guard<std::mutex> locker (debug_mutex);

  debug_filename = filename;
}

}
