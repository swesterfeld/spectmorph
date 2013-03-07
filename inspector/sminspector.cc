// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
#include <sys/time.h>
#include <stdio.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smnavigatorwindow.hh"

#include <QApplication>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  QApplication app (argc, argv);

  if (argc != 2)
    {
      printf ("usage: %s <smindex-file>\n", argv[0]);
      exit (1);
    }

  NavigatorWindow window (argv[1]);
  window.show();

  return app.exec();
}
