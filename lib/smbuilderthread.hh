// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_BUILDER_THREAD_HH
#define SPECTMORPH_BUILDER_THREAD_HH

#include <thread>
#include <mutex>
#include <functional>
#include <vector>

namespace SpectMorph
{

class WavSet;
class WavSetBuilder;

class BuilderThread
{
  std::mutex  mutex;
  std::thread thread;
  bool        thread_quit = false;

  struct Job;

  std::vector<std::unique_ptr<Job>> todo;

  void run();

public:
  BuilderThread();
  ~BuilderThread();

  void   add_job (WavSetBuilder *builder, const std::function<void(WavSet *wav_set)>& done_func);
  size_t job_count();
};

}

#endif
