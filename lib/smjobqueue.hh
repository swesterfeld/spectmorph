/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
