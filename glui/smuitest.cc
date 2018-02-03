#include <stdio.h>
#include "pugl/gl.h"
#if !__APPLE__
#include "GL/glext.h"
#endif
#include "pugl/pugl.h"
#include <unistd.h>
#include <math.h>
#include <string>
#include <vector>
#include <functional>

#include "smwidget.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smwindow.hh"
#include "smmain.hh"

using namespace SpectMorph;

using std::vector;
using std::string;

struct FixedGrid
{
  void add_widget (Widget *w, double x, double y, double width, double height)
  {
    w->x = x * 8;
    w->y = y * 8;
    w->width = width * 8;
    w->height = height * 8;
  }
};

class MainWindow : public Window
{
public:
  MainWindow (int width, int height, PuglNativeWindow win_id = 0) :
    Window (width, height, win_id, true)
  {
    new Label (this, 0, 200, 400, 300, " --- main window --- ");

    vector<string> sl_params { "Skip", "Attack", "Sustain", "Decay", "Release" };
    FixedGrid grid;

    int yoffset = 0;
    for (auto s : sl_params)
      {
        Label *label = new Label (this, 0, 0, 0, 0, s);
        Slider *slider = new Slider (this, 0, 0, 0, 0, 0.0);
        Label *value_label = new Label (this, 0, 0, 0, 0, "50%");

        grid.add_widget (label, 3, yoffset + 3, 10, 2);
        grid.add_widget (slider,  13, yoffset + 3, 25, 2);
        grid.add_widget (value_label, 38, yoffset + 3, 10, 2);
        yoffset += 2;

        slider->set_callback ([=](float value) { value_label->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
      }
  }
};

using std::vector;

#if !VST_PLUGIN
int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  MainWindow window (400, 400);

  window.show();

  while (!window.quit) {
    window.wait_for_event();
    window.process_events();
  }
}
#endif
