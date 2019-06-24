// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplan.hh"
#include "smmain.hh"
#include <assert.h>

using namespace SpectMorph;

using std::string;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

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
