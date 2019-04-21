// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphplanwindow.hh"
#include "smeventloop.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

struct NullSynthInterface : public SynthInterface
{
  Project *m_project;
  Project*
  get_project() override
  {
    return m_project;
  }
  vector<string>
  notify_take_events()
  {
    return {};
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Project project;
  MorphPlanPtr morph_plan = new MorphPlan (project);

  morph_plan->load_default();

  EventLoop event_loop;
  NullSynthInterface nsi;
  nsi.m_project = &project;
  MorphPlanWindow window (event_loop, "SpectMorph - Plan Test", 0, false, morph_plan, &nsi);

  window.control_widget()->set_volume (-6);
  window.connect (window.control_widget()->signal_volume_changed, [](double v) { printf ("volume=%f\n", v); });

  window.show();

  bool quit = false;

  window.set_close_callback ([&]() { quit = true; });

  while (!quit) {
    event_loop.wait_event_fps();
    event_loop.process_events();
  }
}
