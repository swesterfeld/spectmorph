// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlpc.hh"
#include "smmain.hh"
#include <stdio.h>
#include <assert.h>
#include <glib.h>

using namespace SpectMorph;

using std::vector;
using std::complex;
using std::max;

double
rand_pos()
{
  double pos = g_random_double_range (-1.1, 1.1);
  return pos;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  size_t fail = 0;
  double max_diff = 0;
  for (size_t k = 0; k < 1000000; k++)
    {
      fprintf (stderr, "\r%zd %zd %.17g              ", k, fail, max_diff);
      fflush (stderr);
      vector< complex<double> > true_roots;
      size_t n_real = g_random_int_range (0, 3);
      for (size_t i = 0; i < 25; i++)
        {
          if (i < n_real)
            {
              true_roots.push_back (rand_pos());
              true_roots.push_back (rand_pos());
            }
          else
            {
              complex<double> root (rand_pos(), rand_pos());
              true_roots.push_back (root);
              true_roots.push_back (conj (root));
            }
        }

      vector<double> lpc;
      LPC::roots2lpc (true_roots, lpc);

      vector< complex<double> > roots;
      if (!LPC::find_roots (lpc, roots))
        {
          fail++;
        }
      else
        {
          vector<bool> root_used (roots.size());
          for (size_t i = 0; i < roots.size(); i++)
            {
              int best_root = -1;
              double best_diff = 1e32;
              for (size_t j = 0; j < true_roots.size(); j++)
                {
                  if (!root_used[j])
                    {
                      double diff = abs (roots[i] - true_roots[j]);
                      if (diff < best_diff)
                        {
                          best_diff = diff;
                          best_root = j;
                        }
                    }
                }
              if (best_root >= 0)
                {
                  root_used[best_root] = true;
                  max_diff = max (best_diff, max_diff);
                }
              else
                {
                  g_assert_not_reached();
                }
            }
          for (size_t i = 0; i < roots.size(); i++)
            assert (root_used[i]);
        }
    }
#if 0
  for (size_t i = 0; i < roots.size(); i++)
    {
      printf ("%zd %.17g %.17g\n", i, roots[i].real(), roots[i].imag());
    }
#endif
}
