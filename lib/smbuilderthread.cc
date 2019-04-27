// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smbuilderthread.hh"
#include "smwavsetbuilder.hh"
#include <unistd.h>

#include <atomic>

using namespace SpectMorph;

BuilderThread::BuilderThread() :
  thread (&BuilderThread::run, this)
{
}

BuilderThread::~BuilderThread()
{
  kill_all_jobs();

  mutex.lock();
  thread_quit = true;
  mutex.unlock();

  thread.join();
}

struct BuilderThread::Job
{
  std::unique_ptr<WavSetBuilder>       builder;
  std::function<void(WavSet *wav_set)> done_func;
  std::atomic<bool>                    atomic_quit { false };

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
  Job *job = new Job (builder, done_func);

  builder->set_kill_function ([job]() { return job->atomic_quit.load(); });

  std::lock_guard<std::mutex> lg (mutex);
  todo.emplace_back (job);
}

size_t
BuilderThread::job_count()
{
  std::lock_guard<std::mutex> lg (mutex);
  return todo.size();
}

void
BuilderThread::kill_all_jobs()
{
  std::lock_guard<std::mutex> lg (mutex);
  for (auto& job : todo)
    job->atomic_quit.store (true);
}

BuilderThread::Job *
BuilderThread::first_job()
{
  std::lock_guard<std::mutex> lg (mutex);

  if (todo.empty())
    return nullptr;
  else
    return todo[0].get();
}

void
BuilderThread::pop_job()
{
  std::lock_guard<std::mutex> lg (mutex);

  assert (!todo.empty());
  todo.erase (todo.begin());
}

bool
BuilderThread::check_quit()
{
  std::lock_guard<std::mutex> lg (mutex);

  return thread_quit;
}

void
BuilderThread::run()
{
  printf ("BuilderThread: start\n");

  while (!check_quit())
    {
      Job *job = first_job();
      if (job)
        {
          if (!job->atomic_quit.load())
            job->run();

          pop_job();
        }
      else
        {
          // FIXME: sleep on condition instead
          usleep (100 * 1000);
        }
    }
  printf ("BuilderThread: end\n");
}
