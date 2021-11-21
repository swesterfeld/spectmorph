// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smutils.hh"
#include "smmain.hh"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  printf ("string_locale_printf:  %s", string_locale_printf ("%.2f\n", 3.14).c_str());
  printf ("string_printf          %s", string_printf ("%.2f\n", 3.14).c_str());
  printf ("sm_printf              ");
  sm_printf ("%.2f\n", 3.14);
  printf ("printf                 ");
  printf ("%.2f\n", 3.14);
}
