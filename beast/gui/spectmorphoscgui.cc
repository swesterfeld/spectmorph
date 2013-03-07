// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include <QApplication>

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphplanview.hh"
#include "smmorphplanwindow.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "spectmorphoscgui.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

OscGui::OscGui (MorphPlanPtr plan, const string& title) :
  morph_plan (plan)
{
  window = new MorphPlanWindow (morph_plan, title);
  connect (morph_plan.c_ptr(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
  window->show();
}

void
OscGui::on_plan_changed()
{
  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo);
  printf ("%s\n", HexString::encode (data).c_str());
  fflush (stdout);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  QApplication app (argc, argv);

  if (argc != 2)
    {
      printf ("usage: %s <title>\n", argv[0]);
      exit (1);
    }

  // read initial plan
  string plan_str;
  int ch;
  while ((ch = fgetc (stdin)) != '\n')
    plan_str += (char) ch;

  // give parent our pid (for killing the gui)
  printf ("pid %d\n", getpid());
  fflush (stdout);

  MorphPlanPtr morph_plan = new MorphPlan();
  morph_plan->set_plan_str (plan_str);

  OscGui gui (morph_plan, argv[1]);
  int rc = app.exec();

  // let parent know that the user closed the gui window
  printf ("quit\n");
  fflush (stdout);

  return rc;
}
