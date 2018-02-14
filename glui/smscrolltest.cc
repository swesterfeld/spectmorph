// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smscrollview.hh"
#include "smwindow.hh"
#include "smfixedgrid.hh"
#include "smbutton.hh"
#include "smmain.hh"
#include "smscrollbar.hh"

using namespace SpectMorph;

using std::vector;
using std::string;

class MainWindow : public Window
{
public:
  MainWindow() :
    Window (512, 512)
  {
    FixedGrid grid;
    ScrollView *scroll_view = new ScrollView (this);
    grid.add_widget (scroll_view, 2, 2, 40, 40);

    ScrollBar *sb = new ScrollBar (this, 0.5, Orientation::VERTICAL);
    grid.add_widget (sb, 45, 4, 2, 40);
    connect (sb->signal_position_changed, [=](double pos) {
      scroll_view->scroll_y = pos * 900;
    });

    ScrollBar *sbh = new ScrollBar (this, 0.5, Orientation::HORIZONTAL);
    grid.add_widget (sbh, 2, 45, 42, 2);
    connect (sbh->signal_position_changed, [=](double pos) {
      scroll_view->scroll_x = pos * 900;
    });

   for (int bx = 0; bx < 5; bx++)
      {
        for (int by = 0; by < 10; by++)
          {
            string text = string_printf ("Button %dx%d", bx + 1, by + 1);

            Button *button = new Button (scroll_view, text);
            grid.add_widget (button, 3 + bx * 12, 3 + by * 5, 11, 4);

            connect (button->signal_clicked, [=](){ printf ("%s\n", text.c_str()); });
          }
      }
  }
};

using std::vector;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  bool quit = false;

  MainWindow window;

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_for_event();
      window.process_events();
    }
}
