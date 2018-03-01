// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smextfiledialog.hh"
#include "smwindow.hh"

#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace SpectMorph;

using std::string;
using std::vector;

ExtFileDialog::ExtFileDialog (Window *main_window, bool open)
{
  GError *err;

  vector<const char *> argv = { "kdialog", open ? "--getopenfilename" : "--getsavefilename", "/home/stefan", "*.smplan", NULL };
  if (!g_spawn_async_with_pipes (NULL, /* working directory = current dir */
                                 (char **) &argv[0],
                                 NULL, /* inherit environment */
                                 G_SPAWN_SEARCH_PATH,
                                 NULL, NULL, /* no child setup */
                                 &child_pid,
                                 NULL, /* inherit stdin */
                                 &child_stdout,
                                 NULL, /* inherit stderr */
                                 &err))
    {
      printf ("error spawning child %s\n", err->message);
      child_stdout = -1;
      child_pid = -1;
    }
  printf ("child_stdout = %d\n", child_stdout);
  selected_filename_ok = false;
}

void
ExtFileDialog::handle_io()
{
  if (child_stdout >= 0)
    {
      fd_set fds;

      FD_ZERO (&fds);
      FD_SET (child_stdout, &fds);

      timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      int select_ret = select (child_stdout + 1, &fds, NULL, NULL, &tv);

      if (select_ret > 0 && FD_ISSET (child_stdout, &fds))
        {
          char buffer[1024];

          int bytes = read (child_stdout, buffer, 1024);
          for (int i = 0; i < bytes; i++)
            {
              if (buffer[i] >= 32)
                selected_filename += buffer[i];
              if (buffer[i] == '\n')
                selected_filename_ok = true;
            }
          if (bytes == 0)
            {
              // we ignore waitpid result here, as child may no longer exist,
              // and somebody else used waitpid() already (host)
              int status;
              waitpid (child_pid, &status, WNOHANG);

              if (selected_filename_ok)
                signal_file_selected (selected_filename);
              else
                signal_file_selected ("");

              child_pid = -1;

              close (child_stdout);
              child_stdout = -1;
            }
        }
    }
}
