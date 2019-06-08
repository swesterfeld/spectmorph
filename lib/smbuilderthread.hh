// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_BUILDER_THREAD_HH
#define SPECTMORPH_BUILDER_THREAD_HH

#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <condition_variable>

namespace SpectMorph
{

class WavSet;
class WavSetBuilder;

class BuilderThread
{
  std::mutex                mutex;
  std::thread               thread;
  std::condition_variable   cond;
  bool                      thread_quit = false;

  struct Job;

  std::vector<std::unique_ptr<Job>> todo;

  bool check_quit();
  Job *first_job();
  void pop_job();
  void run_job (Job *job);
  void run();

public:
  BuilderThread();
  ~BuilderThread();

  void   add_job (WavSetBuilder *builder, int object_id, const std::function<void(WavSet *wav_set)>& done_func);
  size_t job_count();
  bool   search_job (int object_id);
  void   kill_all_jobs();
  void   kill_jobs_by_id (int object_id);
};

}

#endif
