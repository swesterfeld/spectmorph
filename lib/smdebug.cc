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

namespace
{

struct DebugGlobal
{
  set<string> active_areas;
  FILE       *file = NULL;
  string      filename = "smdebug.log";
  std::mutex  mutex;

  void debugv (const char *area, const char *format, va_list vargs);
};

static DebugGlobal *
debug_global()
{
  static auto instance = new DebugGlobal();   // leak intentionally to avoid global dtor
  return instance;
}

};

static std::atomic<int>  have_areas {0};

void
DebugGlobal::debugv (const char *area, const char *format, va_list vargs)
{
  std::lock_guard<std::mutex> locker (mutex);
  if (active_areas.find (area) != active_areas.end())
    {
      /* we leak an open FILE here, which usually is not a good idea, the only
       * reason why we can do it is that debugging is opt-in (via debug areas),
       * so on an end user installation the file will never be opened
       */
      if (!file)
        {
          char *abs_filename = g_build_filename (sm_get_user_dir (USER_DIR_DATA).c_str(), filename.c_str(), nullptr);
          file = fopen (abs_filename, "w");
          g_free (abs_filename);
        }

      fprintf (file, "%8s | %s", area, string_vprintf (format, vargs).c_str());

      // hack: avoid fflush for encoder (since it writes lots of lines)
      if (strcmp (area, "encoder") != 0)
        fflush (file);
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
  debug_global()->debugv (area, fmt, ap);
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
  debug_global()->debugv ("global", fmt, ap);
  va_end (ap);
}

void
Debug::enable (const std::string& area)
{
  std::lock_guard<std::mutex> locker (debug_global()->mutex);

  debug_global()->active_areas.insert (area);
  have_areas.store (1);
}

bool
Debug::enabled (const std::string& area)
{
  std::lock_guard<std::mutex> locker (debug_global()->mutex);

  return debug_global()->active_areas.find (area) != debug_global()->active_areas.end();
}

void
Debug::set_filename (const std::string& filename)
{
  std::lock_guard<std::mutex> locker (debug_global()->mutex);

  debug_global()->filename = filename;
}

}
