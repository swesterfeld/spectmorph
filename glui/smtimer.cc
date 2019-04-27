// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smtimer.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::Timer");

/* In this implementation, timers belong to a window (not a widget).
 *
 * This means:
 *  - a timer will get deleted if the window gets deleted
 *  - if you want to get rid of the timer earlier, then you can
 *    * use stop() to disable it
 *    * delete the timer object
 *
 * Using 0 as an interval will fire the timer at every single process_events() call.
 */
Timer::Timer (Window *window) :
  window (window)
{
  leak_debugger.add (this);

  window->add_timer (this);
}

Timer::~Timer()
{
  window->remove_timer (this);

  leak_debugger.del (this);
}

void
Timer::process_events()
{
  if (interval_ms >= 0)
    {
      const double last_timestamp = timestamp;
      timestamp = get_time();

      if (timestamp > last_timestamp && timestamp > 0 && last_timestamp > 0)
        {
          running_ms += (timestamp - last_timestamp) * 1000;
          if (running_ms > interval_ms)
            {
              signal_timeout();

              running_ms = 0;
            }
        }
    }
}

void
Timer::start (int ms)
{
  interval_ms = ms;
  running_ms  = 0;
  timestamp   = -1;
}

void
Timer::stop()
{
  interval_ms = -1;
}

bool
Timer::active()
{
  return interval_ms > 0;
}
