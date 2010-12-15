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

#include "smjobqueue.hh"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

using SpectMorph::JobQueue;

void
JobQueue::wait_for_one()
{
  int status;
  pid_t exited = waitpid (-1, &status, 0);

  bool remove = false;

  if (WIFEXITED (status))
    {
      if (WEXITSTATUS (status) != 0)
        all_exit_ok = false;
      remove = true;
    }
  if (WIFSIGNALED (status))
    {
      all_exit_ok = false;
      remove = true;
    }

  if (remove)
    {
      vector<pid_t>::iterator ai = active_pids.begin();
      while (ai != active_pids.end())
        {
          if (*ai == exited)
            {
              ai = active_pids.erase (ai);
            }
          else
            {
              ai++;
            }
        }
    }
}

JobQueue::JobQueue (size_t max_jobs) :
  max_jobs (max_jobs),
  all_exit_ok (true)
{
}

bool
JobQueue::run (const string& cmdline)
{
  while (active_pids.size() + 1 > max_jobs)
    wait_for_one();

  pid_t child_pid = fork();
  if (child_pid < 0)
    return false;

  if (child_pid == 0)
    {
      int status = system (cmdline.c_str());
      if (status < 0)
        exit (127);
      else
        exit (WEXITSTATUS (status));
    }
  else
    {
      active_pids.push_back (child_pid);
    }
  return all_exit_ok;
}

bool
JobQueue::wait_for_all()
{
  while (active_pids.size() > 0)
    wait_for_one();

  return all_exit_ok;
}

JobQueue::~JobQueue()
{
  wait_for_all();
}
