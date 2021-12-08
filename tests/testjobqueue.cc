// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smjobqueue.hh"

using SpectMorph::JobQueue;

int
main()
{
  JobQueue jq;
  for (int i = 0; i < 100; i++)
    {
      jq.run ("true");
    }
}
