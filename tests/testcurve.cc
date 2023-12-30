// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smcurve.hh"
#include "smmain.hh"
#include "smutils.hh"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  Curve curve;
  curve.points.emplace_back (Curve::Point { 0, 0.1, -1 });
  curve.points.emplace_back (Curve::Point { 0.5, 0.5 });
  curve.points.emplace_back (Curve::Point { 1, 0.9 });

  for (int i = -50; i < 1050; i++)
    {
      float f = i / 1000.;
      sm_printf ("%f %f\n", f, curve (f));
    }
}
