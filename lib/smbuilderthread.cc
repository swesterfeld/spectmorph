// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smbuilderthread.hh"
#include "smwavsetbuilder.hh"

#include <atomic>

using namespace SpectMorph;

BuilderThread::BuilderThread()
{
  thread = std::thread (&BuilderThread::run, this);
}

BuilderThread::~BuilderThread()
{
  kill_all_jobs();

  mutex.lock();
  thread_quit = true;
  cond.notify_all();
  mutex.unlock();

  thread.join();
}

struct BuilderThread::Job
{
  std::unique_ptr<WavSetBuilder>       builder;
  int                                  object_id = 0;
  std::function<void(WavSet *wav_set)> done_func;
  std::atomic<bool>                    atomic_quit { false };

  Job (WavSetBuilder *builder, int object_id, const std::function<void(WavSet *wav_set)>& done_func) :
    builder (builder),
    object_id (object_id),
    done_func (done_func)
  {
  }
};

void
BuilderThread::add_job (WavSetBuilder *builder, int object_id, const std::function<void(WavSet *wav_set)>& done_func)
{
  Job *job = new Job (builder, object_id, done_func);

  builder->set_kill_function ([job]() { return job->atomic_quit.load(); });

  std::lock_guard<std::mutex> lg (mutex);
  todo.emplace_back (job);
  cond.notify_all();
}

size_t
BuilderThread::job_count()
{
  std::lock_guard<std::mutex> lg (mutex);
  return todo.size();
}

bool
BuilderThread::search_job (int object_id)
{
  std::lock_guard<std::mutex> lg (mutex);
  for (auto& job : todo)
    if (job->object_id == object_id)
      return true;
  return false;
}

void
BuilderThread::kill_all_jobs()
{
  std::lock_guard<std::mutex> lg (mutex);
  for (auto& job : todo)
    job->atomic_quit.store (true);

  cond.notify_all();
}

void
BuilderThread::kill_jobs_by_id (int object_id)
{
  std::lock_guard<std::mutex> lg (mutex);
  for (auto& job : todo)
    if (job->object_id == object_id)
      job->atomic_quit.store (true);

  cond.notify_all();
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

void
BuilderThread::run_job (Job *job)
{
  if (job->atomic_quit.load())
    return;

  std::unique_ptr<WavSet> wav_set (job->builder->run());

  // use lock to ensure (race-free) that done_func is only executed if job was not killed
  std::lock_guard<std::mutex> lg (mutex);
  if (wav_set && !job->atomic_quit.load())
    job->done_func (wav_set.release());
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
  while (!check_quit())
    {
      Job *job = first_job();
      if (job)
        {
          run_job (job);
          pop_job();
        }
      else
        {
          std::unique_lock<std::mutex> lck (mutex);

          if (!thread_quit)
            cond.wait (lck);
        }
    }
}
