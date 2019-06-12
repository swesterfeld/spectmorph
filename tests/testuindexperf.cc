// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smuserinstrumentindex.hh"
#include "smmain.hh"

#include <stdio.h>
#include <string>

using namespace SpectMorph;
using std::string;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  double start = get_time();

  UserInstrumentIndex uidx;
  int count = 0;
  for (int i = 0; i < 128; i++)
    {
      const string label =  uidx.label (i);
      bool empty = label.size() > 3 && label.substr (label.size() - 3) == "---";
      if (!empty)
        {
          printf ("%s\n", uidx.label (i).c_str());
          count++;
        }
    }

  double end = get_time();
  printf ("%.2f ms for %d items\n\n", (end - start) * 1000, count);
  printf ("%.2f ms for 128 items\n", (end - start) * 1000 / count * 128);
}
