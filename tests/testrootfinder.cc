/*
 * Copyright (C) 2011 Stefan Westerfeld
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

#include "smlpc.hh"
#include "smmain.hh"
#include <stdio.h>
#include <glib.h>

using namespace SpectMorph;

using std::vector;
using std::complex;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  size_t fail = 0;
  for (size_t k = 0; k < 1000000; k++)
    {
      printf ("\r%zd %zd", k, fail);
      fflush (stdout);
      vector< complex<double> > true_roots;
      size_t n_real = g_random_int_range (0, 3);
      for (size_t i = 0; i < 25; i++)
        {
          if (i < n_real)
            {
              true_roots.push_back (g_random_double_range (-1.1, 1.1));
              true_roots.push_back (g_random_double_range (-1.1, 1.1));
            }
          else
            {
              complex<double> root (g_random_double_range (-1.1, 1.1), g_random_double_range (-1.1, 1.1));
              true_roots.push_back (root);
              true_roots.push_back (conj (root));
            }
        }

      vector<double> lpc;
      LPC::roots2lpc (true_roots, lpc);

      vector< complex<double> > roots;
      if (!LPC::find_roots (lpc, roots))
        fail++;
    }
#if 0
  for (size_t i = 0; i < roots.size(); i++)
    {
      printf ("%zd %.17g %.17g\n", i, roots[i].real(), roots[i].imag());
    }
#endif
}
