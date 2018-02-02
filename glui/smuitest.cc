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

class MainWindow : public Window
{
public:
  MainWindow (int width, int height, PuglNativeWindow win_id = 0) :
    Window (width, height, win_id)
  {
    new Label (this, 20, 100, 150, 40, "Attack");
    Slider *s_attack = new Slider (this, 200, 100, 170, 40, 0.0);
    Label  *l_attack_value = new Label (this, 400, 100, 80, 40, "50%");

    new Label (this, 20, 150, 150, 40, "Sustain");
    Slider *s_sustain = new Slider (this, 200, 150, 170, 40, 1.0);
    Label  *l_sustain_value = new Label (this, 400, 150, 80, 40, "50%");

    new Label (this, 0, 200, 512, 300, " --- main window --- ");

    s_attack->set_callback ([=](float value) { l_attack_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
    s_sustain->set_callback ([=](float value) { l_sustain_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
  }
};

using std::vector;

#if !VST_PLUGIN
int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  MainWindow window (512, 512);

  window.show();

  while (!window.quit) {
    window.wait_for_event();
    window.process_events();
  }
}
#endif
