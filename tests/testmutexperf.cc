// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <sys/time.h>
#include <stdio.h>

#include <bse/bsecxxplugin.hh>
#include <QMutex>

#include "smmain.hh"

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  {
    QMutex qmutex;

    // FIRST:
    qmutex.lock();
    qmutex.unlock();

    const unsigned int runs = 1000000;
    double start = gettime();
    for (unsigned int i = 0; i < runs; i++)
      {
        QMutexLocker qlock (&qmutex);
      }
    double end = gettime();
    printf ("%20s %.2f mlocks/sec\n", "QMutex", runs / (end - start) / (1000 * 1000));
  }
#if 0
  {
    Bse::Mutex bmutex;

    // FIRST:
    bmutex.lock();
    bmutex.unlock();

    const unsigned int runs = 1000000;
    double start = gettime();
    for (unsigned int i = 0; i < runs; i++)
      {
        Bse::ScopedLock<Bse::Mutex> block (bmutex);
      }
    double end = gettime();
    printf ("%20s %.2f mlocks/sec\n", "Bse::Mutex", runs / (end - start) / (1000 * 1000));
  }
#endif
}
