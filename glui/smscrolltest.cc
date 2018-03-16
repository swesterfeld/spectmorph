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
using std::max;

class MainWindow : public Window
{
public:
  MainWindow() :
    Window ("SpectMorph - Scroll Test", 512, 512)
  {
    FixedGrid grid;
    ScrollView *scroll_view = new ScrollView (this);
    grid.add_widget (scroll_view, 1, 1, 46, 46);

    Widget *scroll_widget = new Widget (scroll_view);
    int maxx = 0;
    int maxy = 0;

    for (int bx = 0; bx < 10; bx++)
      {
        for (int by = 0; by < 20; by++)
          {
            string text = string_printf ("Button %dx%d", bx + 1, by + 1);

            Button *button = new Button (scroll_widget, text);
            grid.add_widget (button, bx * 12, by * 5, 11, 4);
            maxx = max (maxx, bx * 12 + 11);
            maxy = max (maxy, by * 5 + 4);
            connect (button->signal_clicked, [=](){ printf ("%s\n", text.c_str()); });
          }
      }
    maxx *= 8;
    maxy *= 8;
    scroll_widget->width  = maxx;
    scroll_widget->height = maxy;

    scroll_view->set_scroll_widget (scroll_widget, true, true);
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
