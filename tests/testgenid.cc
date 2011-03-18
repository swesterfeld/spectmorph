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

#include "smmorphplan.hh"
#include <assert.h>

using namespace SpectMorph;

using std::string;

int
main()
{
  string id = MorphPlan::generate_id();
  string id_chars = MorphPlan::id_chars();

  printf ("id = \"%s\"      => bits in id = %.2f\n", id.c_str(), log (pow (MorphPlan::id_chars().size(), id.size())) / log (2));
  printf ("\n");
  printf ("%zd id chars: ", id_chars.size());

  string sort_chars = id_chars;
  sort (sort_chars.begin(), sort_chars.end());
  char last = 0;
  printf ("%s\n", sort_chars.c_str());
  for (string::const_iterator si = sort_chars.begin(); si != sort_chars.end(); si++)
    {
      assert (*si != last);
      last = *si;
    }
  assert (sort_chars.size() == id_chars.size());
}
