// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smbuilderthread.hh"
#include "smwavsetbuilder.hh"
#include <unistd.h>

using namespace SpectMorph;

BuilderThread::BuilderThread() :
  thread (&BuilderThread::run, this)
{
}

BuilderThread::~BuilderThread()
{
  mutex.lock();
  thread_quit = true;
  mutex.unlock();

  thread.join();
}

struct BuilderThread::Job
{
  std::unique_ptr<WavSetBuilder>       builder;
  std::function<void(WavSet *wav_set)> done_func;

  Job (WavSetBuilder *builder, const std::function<void(WavSet *wav_set)>& done_func) :
    builder (builder),
    done_func (done_func)
  {
  }

  void
  run()
  {
    WavSet *wav_set = builder->run();

    if (wav_set)
      done_func (wav_set);
  }
};

void
BuilderThread::add_job (WavSetBuilder *builder, const std::function<void(WavSet *wav_set)>& done_func)
{
  builder->set_kill_function ([this]() {
    std::lock_guard<std::mutex> lg (mutex);
    return thread_quit;
  });
  mutex.lock();
  todo.emplace_back (new Job (builder, done_func));
  mutex.unlock();
}

size_t
BuilderThread::job_count()
{
  std::lock_guard<std::mutex> lg (mutex);
  return todo.size();
}

void
BuilderThread::run()
{
  printf ("BuilderThread: start\n");
  bool quit;
  while (!quit)
    {
      // FIXME: sleep on condition instead
      usleep (100 * 1000);

      mutex.lock();
      quit = thread_quit;
      if (!quit && todo.size())
        {
          Job *job = todo[0].get();
          mutex.unlock();
          job->run();
          mutex.lock();
          todo.erase (todo.begin());
        }
      mutex.unlock();
    }
  printf ("BuilderThread: end\n");
}
