// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <vector>
#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <assert.h>

#include "smmain.hh"
#include "smlpc.hh"

using std::vector;
using namespace SpectMorph;

int
main (int argc, char **argv) // generate and dump coefficients a_1, a_2, ..., a_K from signal on stdin
{
  sm_init (&argc, &argv);

  assert (argc == 2);

  //----------------------- parse signal from stdin -----------------------------
  char buffer[1024];
  vector<float> signal;

  int line = 1;
  while (fgets (buffer, 1024, stdin) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          char *end_ptr;
          bool parse_error = false;

          signal.push_back (g_ascii_strtod (buffer, &end_ptr));

          // chech that we parsed at least one character
          parse_error = parse_error || (end_ptr == buffer);

          // check that we parsed the whole line
          while (*end_ptr == ' ' || *end_ptr == '\n' || *end_ptr == '\t' || *end_ptr == '\r')
            end_ptr++;
          parse_error = parse_error || (*end_ptr != 0);

          if (parse_error)
            {
              g_printerr ("testlpca: parse error on line %d\n", line);
              exit (1);
            }
        }
      line++;
    }
  //----------------------- generate lpc coeffs -----------------------------
  const int lpc_order = atoi (argv[1]);

  vector<double> lpc (lpc_order);
  LPC::compute_lpc (lpc, &signal[0], &signal[signal.size()]);

  for (auto d : lpc)
    {
      printf ("%.17g\n", d);
    }
}
