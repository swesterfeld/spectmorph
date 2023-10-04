// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smlabel.hh"

namespace SpectMorph
{

class ClickableLabel : public Label
{
  bool pressed = false;
public:
  ClickableLabel (Widget *parent, const std::string& text) :
    Label (parent, text)
  {
  }
  void
  enter_event() override
  {
    set_bold (true);
    update();
  }
  void
  leave_event() override
  {
    set_bold (false);
    update();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON)
      {
        pressed = true;
        update();
      }
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button != LEFT_BUTTON || !pressed)
      return;

    pressed = false;
    update();

    if (event.x >= 0 && event.y >= 0 && event.x < width() && event.y < height())
      signal_clicked();
  }
  Signal<> signal_clicked;
};

}
