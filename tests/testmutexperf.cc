// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <sys/time.h>
#include <stdio.h>

#include <QMutex>

#include "smmain.hh"
#include "smutils.hh"

#if SPECTMORPH_HAVE_BSE
#include <bse/bsecxxplugin.hh>
#endif

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

using namespace SpectMorph;

template<class MTest>
void run_test (const char *label)
{
  MTest mtest;

  // FIRST:
  mtest.do_lock();

  const unsigned int runs = 1000000;
  double start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    mtest.do_lock();
  double end = gettime();

  printf ("%20s %.2f mlocks/sec\n", label, runs / (end - start) / (1000 * 1000));
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  struct QMTest {
    QMutex mutex;
    void do_lock() { QMutexLocker qlock (&mutex); }
  };
  run_test<QMTest> ("QMutex");

  struct SMTest {
    std::mutex mutex;
    void do_lock() { std::lock_guard<std::mutex> guard (mutex); }
  };
  run_test<SMTest> ("std::mutex");

#if SPECTMORPH_HAVE_BSE
  struct BMTest {
    Bse::Mutex mutex;
    void do_lock() { Bse::ScopedLock<Bse::Mutex> block (mutex); }
  };
  run_test<BMTest> ("Bse::Mutex");
#endif
}
