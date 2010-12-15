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
