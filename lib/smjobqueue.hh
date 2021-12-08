// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_JOB_QUEUE_HH
#define SPECTMORPH_JOB_QUEUE_HH

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <vector>

namespace SpectMorph
{

class JobQueue
{
  size_t             max_jobs;
  std::vector<pid_t> active_pids;
  bool               all_exit_ok;

  void wait_for_one();

public:
  JobQueue (size_t max_jobs = 1);

  bool run (const std::string& cmdline);
  bool wait_for_all();

  ~JobQueue();
};

} // namespace SpectMorph

#endif
