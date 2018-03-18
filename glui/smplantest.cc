// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphplanwindow.hh"

using namespace SpectMorph;

using std::string;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  MorphPlanPtr morph_plan = new MorphPlan;

  string filename = sm_get_default_plan();

  GenericIn *in = StdioIn::open (filename);
  if (in)
    {
      morph_plan->load (in);
      delete in;
    }
  else
    {
      fprintf (stderr, "Error opening '%s'.\n", filename.c_str());
    }

  MorphPlanWindow window ("SpectMorph - Plan Test", 0, false, morph_plan);

  window.control_widget()->set_volume (-6);
  window.connect (window.control_widget()->signal_volume_changed, [](double v) { printf ("volume=%f\n", v); });

  window.show();

  bool quit = false;

  window.set_close_callback ([&]() { quit = true; });

  while (!quit) {
    window.wait_for_event();
    window.process_events();
  }
}
