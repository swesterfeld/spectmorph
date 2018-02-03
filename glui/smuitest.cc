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
#include "smframe.hh"
#include "smcombobox.hh"

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
    Label *mw_label = new Label (this, 0, 200, 400, 200, " --- main window --- ");
    mw_label->align = Label::Align::CENTER;

    vector<string> sl_params { "Skip", "Attack", "Sustain", "Decay", "Release" };
    FixedGrid grid;

    grid.add_widget (new Frame (this, 0, 0, 0, 0), 1, 1, 43, sl_params.size() * 2 + 11);

    Label *op_title = new Label (this, 0, 0, 0, 0, "Output: Output #1");
    op_title->align = Label::Align::CENTER;
    grid.add_widget (op_title, 1, 1, 43, 4);

    grid.add_widget (new Label (this, 0, 0, 0, 0, "LSource"), 3, 5, 7, 3);
    grid.add_widget (new ComboBox (this), 10, 5, 32, 3);
    grid.add_widget (new Label (this, 0, 0, 0, 0, "RSource"), 3, 8, 7, 3);
    grid.add_widget (new ComboBox (this), 10, 8, 32, 3);
    int yoffset = 11;
    for (auto s : sl_params)
      {
        Label *label = new Label (this, 0, 0, 0, 0, s);
        Slider *slider = new Slider (this, 0, 0, 0, 0, 0.0);
        Label *value_label = new Label (this, 0, 0, 0, 0, "50%");

        grid.add_widget (label, 3, yoffset, 7, 2);
        grid.add_widget (slider,  10, yoffset, 27, 2);
        grid.add_widget (value_label, 38, yoffset, 5, 2);
        yoffset += 2;

        slider->set_callback ([=](float value) { value_label->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
      }

    if (0) // TEXT ALIGN
      {
        vector<string> texts = { "A", "b", "c", "D", ".", "'", "|" };
        for (size_t x = 0; x < texts.size(); x++)
          grid.add_widget (new Label (this, 0, 0, 0, 0, texts[x]), 3 + x * 2, 20, 2, 2);
      }
  }
};

using std::vector;

#if !VST_PLUGIN
int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  MainWindow window (360, 360);

  window.show();

  while (!window.quit) {
    window.wait_for_event();
    window.process_events();
  }
}
#endif
