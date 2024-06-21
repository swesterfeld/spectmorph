// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smproject.hh"

#include <unistd.h>

using std::vector;

using namespace SpectMorph;

ControlEventVector cv;
std::atomic<int>   active_threads {0};
int                max_active_threads;

void
f (bool lock)
{
  for (int i = 0; i < 100; i++)
    {
      if (lock)
        {
          while (!cv.try_lock())
            usleep (10 * 1000);
        }
      active_threads++;
      max_active_threads = std::max (max_active_threads, active_threads.load());
      usleep (1000);
      active_threads--;
      if (lock)
        cv.unlock();
    }
}

int
main()
{
  for (auto lock : { false, true })
    {
      vector<std::thread> v;

      max_active_threads = 0;

      for (int n = 0; n < 10; n++)
        v.emplace_back (f, lock);

      for (auto& t : v)
        t.join();
      printf ("lock: %s - max_active_threads: %d\n", lock ? "#T" : "#F", max_active_threads);

      if (lock)
        assert (max_active_threads == 1);
      else
        assert (max_active_threads == 10);
    }
}
